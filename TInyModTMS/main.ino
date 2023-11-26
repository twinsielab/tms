#include "_types.h"
#include "_config.h"

#define UNPRESSED !SELECTOR_DETECT_LEVEL
#define MOTOR_OFF !MOTOR_ON


// Function to check if a slot is loaded
bool isSelectorLoaded() {
  // Read the selector switch pin state
  bool state = digitalRead(SELECTOR_END_PIN);
  return (state == SELECTOR_DETECT_LEVEL);
}

// Function to check if a slot is loaded
bool isSlotLoaded(uint8_t slotNumber) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return false;
  // Read the selector switch pin state
  bool state = digitalRead(slots[slotNumber - 1].selectorSwitchPin);
  return (state == SELECTOR_DETECT_LEVEL);
}

// Check which slot is currently loaded, 0 if none
uint8_t loadedSlot() {
  for (uint8_t i = 1; i <= MAX_SLOTS; i++) {
    if (isSlotLoaded(i)) {
      return i;  // Return the number of the loaded slot
    }
  }
  return 0;  // Return 0 if no slots are loaded
}

// Function to move a motor associated with a given slot
void moveMotor(uint8_t slotNumber, float moveDistance, float speed) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

  Slot slot = slots[slotNumber - 1];

  // Calculate the number of steps and step delay
  unsigned long steps = abs(moveDistance * slot.stepsPerMM);      // Convert distance to steps
  unsigned int stepDelay = 1000000 / (speed * slot.stepsPerMM);  // Calculate delay in microseconds

  // Determine direction
  bool dir = (moveDistance >= 0) ? slot.feedDir : !slot.feedDir;
  digitalWrite(slot.dirPin, dir);

  // Enable motor
  digitalWrite(slot.enablePin, MOTOR_ON);

  // Perform steps with delay
  for (unsigned long i = 0; i < steps; i++) {
    digitalWrite(slot.stepPin, HIGH);
    delayMicroseconds(stepDelay);
    digitalWrite(slot.stepPin, LOW);
    delayMicroseconds(stepDelay);
  }

  // Disable motor
  // digitalWrite(slot.enablePin, MOTOR_OFF);
}


void unloadSlot(uint8_t slotNumber) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

  uint8_t currentLoadedSlot = loadedSlot();  // Check if any slot is currently loaded
  if (currentLoadedSlot != slotNumber) {
    Serial.println("[SLOT-" + String(slotNumber) + "] Already unloaded!");
    return;
  }

  Serial.println("[SLOT-" + String(slotNumber) + "] Unloading...");

  // Load by backward forward until unloaded
  while (!isSelectorLoaded()) {
    moveMotor(slotNumber, 1, LOAD_SPEED);  // Move by 1mm at LOAD_SPEED
  }

  // Unload by moving backward until not loaded
  while (isSlotLoaded(slotNumber)) {
    moveMotor(slotNumber, -1, UNLOAD_SPEED);  // Move backward by 1mm at speed 10mm/s
  }

  // retract this amount past the switch
  moveMotor(slotNumber, -SELECTOR_OFFSET_BEFORE, UNLOAD_SPEED);

  // Turn motor off
  digitalWrite(slots[slotNumber-1].enablePin, MOTOR_OFF);

  Serial.println("[SLOT-" + String(slotNumber) + "] Unloaded!");
}


void loadSlot(uint8_t slotNumber) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

  uint8_t currentLoadedSlot = loadedSlot();  // Check if any slot is currently loaded
  if (currentLoadedSlot == 0) {
    Serial.println("[SLOT-" + String(slotNumber) + "] Loading...");

    // Load by moving forward until loaded
    while (!isSlotLoaded(slotNumber)) {
      moveMotor(slotNumber, 1, LOAD_SPEED);  // Move by 1mm at LOAD_SPEED
    }

    // Load by moving forward until loaded
    while (!isSelectorLoaded()) {
      moveMotor(slotNumber, 1, LOAD_SPEED);  // Move by 1mm at LOAD_SPEED
    }
    
    // Move this amount past the selector output
    moveMotor(slotNumber, SELECTOR_OFFSET_AFTER, LOAD_SPEED);
    Serial.println("[SLOT-" + String(slotNumber) + "] Loaded!");
  }
  else if (currentLoadedSlot == slotNumber) {
    Serial.println("[SLOT-" + String(slotNumber) + "] Already loaded!");
    return;
  }
  else if (currentLoadedSlot != slotNumber) {
    Serial.println("[SLOT-" + String(slotNumber) + "] Cant! "+ String(currentLoadedSlot) + " is currently loaded!");
    return;
  }
}


void filamentSwap(uint8_t slotNumber) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

  uint8_t currentLoadedSlot = loadedSlot();  // Check if any slot is currently loaded

  // Unload the current loaded slot if one is loaded
  if (currentLoadedSlot != 0 && currentLoadedSlot != slotNumber) {
    // Unload current loaded slot
    unloadSlot(currentLoadedSlot);
  }

  // Load the specified slot
  loadSlot(slotNumber);

  Serial.println("Swap complete.");
}


void feed() {

  
  while (true) {
    uint8_t currentLoadedSlot = loadedSlot();

    // feed until it finds resistance in the buffed (unclick switch)
    Serial.print("[Buffer] Feeding");
    while (digitalRead(BUFFER_PIN) == BUFFER_EMPTY_LEVEL) {
      moveMotor(currentLoadedSlot, 1, FEED_SPEED);  // Move forward by 1mm at FEED_SPEED
      Serial.print(".");
    }
    Serial.println("");

    // compress the spring by fixed amount
    Serial.println("[Buffer] Preloading...");
    moveMotor(currentLoadedSlot, BUFFER_PRELOAD_LENGH, PRELOAD_SPEED);

    Serial.println("[Buffer] Idle");
    while (digitalRead(BUFFER_PIN) == !BUFFER_EMPTY_LEVEL); // wait for buffer to be empty
  }
}



void setup() {
  Serial.begin(9600);
  Serial.println("TMS BOOT");

  // Configure pins for each slot
  Serial.println("Initializing Slots... " + String(MAX_SLOTS));
  for (uint8_t i = 0; i < MAX_SLOTS; i++) {
    pinMode(slots[i].enablePin, OUTPUT);
    pinMode(slots[i].dirPin, OUTPUT);
    pinMode(slots[i].stepPin, OUTPUT);
    pinMode(slots[i].selectorSwitchPin, INPUT_PULLUP);

    // Initialize enable pin to LOW (assuming active HIGH - motor disabled initially)
    digitalWrite(slots[i].enablePin, !MOTOR_ON);

    Serial.println("[SLOT-" + String(i+1) + "] " + "initialized!");
  }

  Serial.println("Initializing Selector... ");
  pinMode(SELECTOR_END_PIN, INPUT_PULLUP);

  Serial.println("Initializing Buffer... ");
  pinMode(BUFFER_PIN, INPUT_PULLUP);
  Serial.println("[BUFFER] initialized!");



  Serial.println("[TMS] running self check...");
  // self check
  // Check which slot is currently loaded, 0 if none
  int selectorLoadedCount = 0;
  for (uint8_t i = 1; i <= MAX_SLOTS; i++) {
    if (isSlotLoaded(i)) {
      selectorLoadedCount++;
      Serial.println("[SELECTOR] Input " + String(i+1) + " has filament");
    }
  }

  if (selectorLoadedCount==0) {
    Serial.println("[SELECTOR] No filament is loaded!");
  }
  else if (selectorLoadedCount>1) {
    Serial.println("[SELECTOR] FAILED! More then 1 slot is reporting loaded!");
  }
  
  bool isSelectorLoaded = digitalRead(SELECTOR_END_PIN) == SELECTOR_DETECT_LEVEL;
  if (isSelectorLoaded) Serial.print("[SELECTOR] Loaded: ");
  if (isSelectorLoaded) Serial.println("Yes");
  else Serial.println("No");
  
  if (selectorLoadedCount==0 && isSelectorLoaded) {
    Serial.println("[SELECTOR] FAILED! All the inputs are empty but the Selector output has filament!");
  }
  else Serial.println("[SELECTOR] Self check: PASS!");


  if (digitalRead(BUFFER_PIN)==!BUFFER_EMPTY_LEVEL && selectorLoadedCount==0) {
    Serial.println("FAILED! Buffer is jammed or one of the Selector filament detectors isn't detecting filament!");
  }
  else Serial.println("[BUFFER] Self check: PASS!");


  Serial.println("TMS Ready!");
}

void loop() {
  if (Serial.available() > 0) {

    // Read the incoming command
    String command = Serial.readStringUntil('\n');
    command.toUpperCase();  // Convert the command to uppercase for case-insensitive comparison

    // MOVE <slot> <length> <speed>
    // Move a motor slot by a certain amount
    if (command.startsWith("MOVE ")) {
      int firstSpace = command.indexOf(' ', 5);
      int secondSpace = command.indexOf(' ', firstSpace + 1);
      // Extracting slot number, length, and speed
      int slotNumber = command.substring(5, firstSpace).toInt();
      float length = command.substring(firstSpace + 1, secondSpace).toFloat();
      float speed = command.substring(secondSpace + 1).toFloat();
      if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

      Serial.println("Moving Feeder " + String(slotNumber) + " by " + String(length) + "mm at " + String(speed) + "mm/s");

      moveMotor(slotNumber, length, speed);
      Serial.println("End.");
    }

    // SWAP <slot>
    // Do a complete filament swap and handles unloading other slots if necessary
    else if (command.startsWith("SWAP ")) {
      int firstSpace = command.indexOf(' ', 5);
      int slotNumber = command.substring(5, firstSpace).toInt();
      if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

      filamentSwap(slotNumber);
      Serial.println("End.");
    }

    // LOAD <slot>
    // Loads a slot
    else if (command.startsWith("LOAD ")) {
      int firstSpace = command.indexOf(' ', 5);
      int slotNumber = command.substring(5, firstSpace).toInt();
      if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

      loadSlot(slotNumber);
      Serial.println("End.");
    }

    // UNLOAD <slot>
    // Unloads a slot
    else if (command.startsWith("UNLOAD ")) {
      int firstSpace = command.indexOf(' ', 7);
      int slotNumber = command.substring(7, firstSpace).toInt();
      if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

      unloadSlot(slotNumber);
      Serial.println("End.");
    }

    // FEED
    // After loaded this will keep the buffer fed at all times
    else if (command.startsWith("FEED")) {
      Serial.println(("Entering feed mode (You need to reset to stop)"));
      feed();
    }

    else {
      Serial.println("Invalid command");
    }
  }
}
