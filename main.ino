#include "_types.h"
#include "_config.h"

#define UNPRESSED !PRESSED

// Function to check if a slot is loaded
bool isSlotLoaded(int slotNumber) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return false;
  // Read the hub switch pin state
  int state = digitalRead(slots[slotNumber - 1].hubSwitchPin);
  return (state == PRESSED);
}

// Check which slot is currently loaded, 0 if none
int loadedSlot() {
  for (int i = 1; i <= MAX_SLOTS; i++) {
    if (isSlotLoaded(i)) {
      return i;  // Return the number of the loaded slot
    }
  }
  return 0;  // Return 0 if no slots are loaded
}

// Function to move a motor associated with a given slot
void moveMotor(int slotNumber, float moveDistance, float speed) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

  Slot slot = slots[slotNumber - 1];

  // Calculate the number of steps and step delay
  int steps = abs(moveDistance * slot.stepsPerMM);      // Convert distance to steps
  int stepDelay = 1000000 / (speed * slot.stepsPerMM);  // Calculate delay in microseconds

  // Determine direction
  bool dir = (moveDistance >= 0) ? slot.feedDir : !slot.feedDir;
  digitalWrite(slot.dirPin, dir);

  // Enable motor if necessary
  digitalWrite(slot.enablePin, MOTOR_ON);

  // Perform steps with delay
  for (int i = 0; i < steps; i++) {
    digitalWrite(slot.stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(slot.stepPin, LOW);
    delayMicroseconds(stepDelay);
  }

  // Disable motor if necessary
  digitalWrite(slot.enablePin, !MOTOR_ON);
}


void unloadSlot(int slotNumber) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

  int currentLoadedSlot = loadedSlot();  // Check if any slot is currently loaded
  if (currentLoadedSlot == 0) {
    Serial.println("No filament currently loaded!");
    return;
  }
  if (currentLoadedSlot != slotNumber) {
    Serial.println("Slot " + String(slotNumber) + " is already unloaded!");
    return;
  }

  Serial.println("Unloading slot " + String(slotNumber) + "...");
  // Unload by moving backward until not loaded
  while (isSlotLoaded(slotNumber)) {
    moveMotor(slotNumber, -1, LOAD_SPEED);  // Move backward by 1mm at speed 10mm/s
  }
  Serial.println("Unloaded.");
}


void loadSlot(int slotNumber) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");


  int currentLoadedSlot = loadedSlot();  // Check if any slot is currently loaded
  if (currentLoadedSlot == slotNumber) {
    Serial.println("Slot " + String(currentLoadedSlot) + " is already loaded!");
    return;
  }
  if (currentLoadedSlot != slotNumber) {
    Serial.println("Can't load! Slot " + String(currentLoadedSlot) + " is currently loaded!");
    return;
  }

  Serial.println("Loading slot " + String(currentLoadedSlot) + "...");
  // Load by moving backward until not loaded
  while (isSlotLoaded(currentLoadedSlot)) {
    moveMotor(currentLoadedSlot, 1, LOAD_SPEED);  // Move backward by 1mm at speed 10mm/s
  }
  Serial.println("Loaded.");
}


void filamentSwap(int slotNumber) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

  int currentLoadedSlot = loadedSlot();  // Check if any slot is currently loaded

  // Unload the current loaded slot if one is loaded
  if (currentLoadedSlot == -1) {
    Serial.println("No filament currently loaded!");
  }
  if (currentLoadedSlot == slotNumber) {
    Serial.println("Slot " + String(currentLoadedSlot) + " is already loaded!");
  } else if (currentLoadedSlot != -1 && currentLoadedSlot != slotNumber) {
    // Unload current loaded slot
    unloadSlot(currentLoadedSlot);
  }

  // Load the specified slot
  loadSlot(slotNumber);

  // Feed 100mm of filament, past the hub
  moveMotor(slotNumber, 100, LOAD_SPEED);  // Load 100mm at speed 10mm/s

  Serial.println("Swap complete.");
}


void feed() {

  while (true) {
    int currentLoadedSlot = loadedSlot();

    // feed until it finds resistance in the buffed (unclick switch)
    Serial.print("[Buffer] Feeding");
    while (digitalRead(BUFFER_PIN) == PRESSED) {
      moveMotor(currentLoadedSlot, 1, LOAD_SPEED);  // Move forward by 1mm at speed 10mm/s
      Serial.print(".");
    }
    Serial.println("");

    // compress the spring by 10mm
    Serial.println("[Buffer] Preloading...");
    moveMotor(currentLoadedSlot, 10, LOAD_SPEED);

    Serial.println("[Buffer] Idle");
    while (digitalRead(BUFFER_PIN) == UNPRESSED); // wait for buffer to be empty
  }
}



void setup() {
  Serial.begin(9600);
  Serial.println("TMS BOOT");

  // Configure pins for each slot
  Serial.println("Initializing Slots... " + String(MAX_SLOTS));
  for (int i = 0; i < MAX_SLOTS; i++) {
    pinMode(slots[i].enablePin, OUTPUT);
    pinMode(slots[i].dirPin, OUTPUT);
    pinMode(slots[i].stepPin, OUTPUT);
    pinMode(slots[i].hubSwitchPin, INPUT_PULLUP);

    // Initialize enable pin to LOW (assuming active HIGH - motor disabled initially)
    digitalWrite(slots[i].enablePin, !MOTOR_ON);

    Serial.println("[Slot-" + String(i+1) + "] " + "initialized!");
  }


  Serial.println("Initializing Buffer... ");
  pinMode(BUFFER_PIN, INPUT_PULLUP);
  Serial.println("[Buffer] initialized!");
}
void loop() {
  if (Serial.available() > 0) {

    // Read the incoming command
    String command = Serial.readStringUntil('\n');
    command.toUpperCase();  // Convert the command to uppercase for case-insensitive comparison

    // MOVE <slot> <length> <speed>
    // Move a motor slot by a certain amount
    if (command.startsWith("MOVE ")) {
      // Splitting the command string
      int firstSpace = command.indexOf(' ', 5);
      int secondSpace = command.indexOf(' ', firstSpace + 1);

      // Extracting slot number, length, and speed
      int slotNumber = command.substring(5, firstSpace).toInt();
      float length = command.substring(firstSpace + 1, secondSpace).toFloat();
      float speed = command.substring(secondSpace + 1).toFloat();

      if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

      Serial.println("Moving Feeder " + String(slotNumber) + " with length " + String(length) + "mm at speed " + String(speed) + "mm/s");

      moveMotor(slotNumber, length, speed);
      Serial.println("End.");
    }

    // SWAP <slot>
    // Do a complete filament swap and handles unloading other slots if necessary
    else if (command.startsWith("SWAP ")) {
      // Splitting the command string
      int firstSpace = command.indexOf(' ', 5);

      // Extracting slot number
      int slotNumber = command.substring(5, firstSpace).toInt();

      if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

      Serial.println("Loading " + String(slotNumber));

      filamentSwap(slotNumber);
      Serial.println("End.");
    }

    // LOAD <slot>
    // Loads a slot
    else if (command.startsWith("LOAD ")) {
      // Splitting the command string
      int firstSpace = command.indexOf(' ', 5);

      // Extracting slot number
      int slotNumber = command.substring(5, firstSpace).toInt();

      if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

      Serial.println("Loading " + String(slotNumber) + "...");

      loadSlot(slotNumber);

      Serial.println("End.");
    }

    // UNLOAD <slot>
    // Unloads a slot
    else if (command.startsWith("UNLOAD ")) {
      // Splitting the command string
      int firstSpace = command.indexOf(' ', 7);

      // Extracting slot number
      int slotNumber = command.substring(7, firstSpace).toInt();

      if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

      Serial.println("Unloading " + String(slotNumber) + "...");

      unloadSlot(slotNumber);
      Serial.println("End.");
    }

    // FEED
    // After loaded this will keep the buffer fed at all times
    else if (command.startsWith("FEED")) {
      Serial.println("Entering feed mode (You need to reset to stop)");
      feed();
    }

    // ... Handle other commands ...
    else {
      Serial.println("Invalid command");
    }
  }
}
