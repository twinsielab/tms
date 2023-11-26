#include "types.h"
#include "config.h"

#define MOTOR_OFF !MOTOR_ON

namespace Selector {

  // Function to check if the selector is fully loaded
  bool isLoaded() {
    // Read the selector switch pin state
    bool state = digitalRead(SELECTOR_END_PIN);
    return (state == SELECTOR_DETECT_LEVEL);
  }

  // Function to check if a slot is loaded
  bool inputHasFilament(uint8_t slotNumber) {
    if (slotNumber < 1 || slotNumber > MAX_SLOTS) return false;
    // Read the selector switch pin state
    bool state = digitalRead(slots[slotNumber - 1].selectorInputPin);
    return (state == SELECTOR_DETECT_LEVEL);
  }

  // Check which slot is currently loaded, 0 if none
  uint8_t getLoadedInput() {
    for (uint8_t i = 1; i <= MAX_SLOTS; i++) {
      if (inputHasFilament(i)) {
        return i;  // Return the number of the loaded slot
      }
    }
    return 0;  // Return 0 if no slots are loaded
  }
}

// Function to move a motor associated with a given slot
void moveFeederMotor(uint8_t slotNumber, float moveDistance, float speed) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

  Slot slot = slots[slotNumber - 1];

  // Calculate the number of steps and step delay
  unsigned long steps = abs(moveDistance * slot.feederStepsPerMM);     // Convert distance to steps
  unsigned int stepDelay = 1000000 / (speed * slot.feederStepsPerMM);  // Calculate delay in microseconds

  // Determine direction
  bool dir = (moveDistance >= 0) ? slot.feedDir : !slot.feedDir;
  digitalWrite(slot.feederDirPin, dir);

  // Enable motor
  digitalWrite(slot.feederEnablePin, MOTOR_ON);

  // Perform steps with delay
  bool s = LOW;
  for (unsigned long i = 0; i < steps; i++) {
    s = !s;
    digitalWrite(slot.feederStepPin, s);
    delayMicroseconds(stepDelay);
  }

  // Disable motor
  // digitalWrite(slot.feederEnablePin, MOTOR_OFF);
}

// Function to move a motor associated with a given slot
void moveSpoolMotor(uint8_t slotNumber, float moveDistance, float speed) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

  Slot slot = slots[slotNumber - 1];

  // Calculate the number of steps and step delay
  unsigned long steps = abs(moveDistance * slot.spoolStepsPerMM);     // Convert distance to steps
  unsigned int stepDelay = 1000000 / (speed * slot.spoolStepsPerMM);  // Calculate delay in microseconds

  // Determine direction
  bool dir = (moveDistance >= 0) ? slot.spoolDir : !slot.spoolDir;
  digitalWrite(slot.spoolDirPin, dir);

  // Enable motor
  digitalWrite(slot.spoolEnablePin, MOTOR_ON);

  // Perform steps with delay
  bool s = LOW;
  for (unsigned long i = 0; i < steps; i++) {
    s = !s;
    digitalWrite(slot.spoolStepPin, s);
    delayMicroseconds(stepDelay);
  }

  // Disable motor
  digitalWrite(slot.spoolEnablePin, MOTOR_OFF);
}


void moveFeederAndSpoolMotor(uint8_t slotNumber, float moveDistance, float speed) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

  Slot slot = slots[slotNumber - 1];

  // Calculate the number of steps for feeder and spool
  unsigned long feederSteps = abs(moveDistance * slot.feederStepsPerMM);      
  unsigned long spoolSteps = abs(moveDistance * slot.spoolStepsPerMM);      

  // Determine direction for both motors
  bool feederDir = (moveDistance >= 0) ? slot.feedDir : !slot.feedDir;
  bool spoolDir = (moveDistance >= 0) ? slot.spoolDir : !slot.spoolDir;
  
  // Set direction
  digitalWrite(slot.feederDirPin, feederDir);
  digitalWrite(slot.spoolDirPin, spoolDir);

  // Enable both motors
  digitalWrite(slot.feederEnablePin, MOTOR_ON);
  digitalWrite(slot.spoolEnablePin, MOTOR_ON);

  // Calculate total movement time in microseconds
  unsigned long totalTime = abs(moveDistance) / speed * 1000000;

  // Calculate step intervals for each motor
  unsigned long feederStepInterval = totalTime / feederSteps;
  unsigned long spoolStepInterval = totalTime / spoolSteps;

  unsigned long feederNextStepTime = 0;
  unsigned long spoolNextStepTime = 0;

  bool feederStepState = LOW;
  bool spoolStepState = LOW;

  unsigned long startTime = micros();
  while (feederSteps > 0 || spoolSteps > 0) {
    unsigned long currentTime = micros() - startTime;
    
    if (feederSteps > 0 && currentTime >= feederNextStepTime) {
      feederStepState = !feederStepState;
      digitalWrite(slot.feederStepPin, feederStepState);
      feederNextStepTime += feederStepInterval;
      feederSteps--;
    }

    if (spoolSteps > 0 && currentTime >= spoolNextStepTime) {
      spoolStepState = !spoolStepState;
      digitalWrite(slot.spoolStepPin, spoolStepState);
      spoolNextStepTime += spoolStepInterval;
      spoolSteps--;
    }
  }

  // Disable both motors
  // digitalWrite(slot.feederEnablePin, MOTOR_OFF);
  digitalWrite(slot.spoolEnablePin, MOTOR_OFF);
}





void unloadSlot(uint8_t slotNumber) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

  uint8_t currentLoadedSlot = Selector::getLoadedInput();  // Check if any slot is currently loaded
  if (currentLoadedSlot != slotNumber) {
    Serial.println("[SLOT-" + String(slotNumber) + "] Already unloaded!");
    return;
  }

  Serial.println("[SLOT-" + String(slotNumber) + "] Unloading...");

  // Load by backward forward until unloaded
  while (!Selector::isLoaded()) {
    moveFeederAndSpoolMotor(slotNumber, MOVE_READ_DISTANCE, LOAD_SPEED);  // Move by 1mm at LOAD_SPEED
    // moveFeederMotor(slotNumber, MOVE_READ_DISTANCE, LOAD_SPEED);  // Move by 1mm at LOAD_SPEED
    // moveSpoolMotor(slotNumber, MOVE_READ_DISTANCE, LOAD_SPEED);  // Move by 1mm at LOAD_SPEED
  }

  // Unload by moving backward until not loaded
  while (Selector::inputHasFilament(slotNumber)) {
    moveFeederAndSpoolMotor(slotNumber, -MOVE_READ_DISTANCE, UNLOAD_SPEED);  // Move backward by 1mm at speed 10mm/s
    // moveFeederMotor(slotNumber, -MOVE_READ_DISTANCE, UNLOAD_SPEED);  // Move backward by 1mm at speed 10mm/s
    // moveSpoolMotor(slotNumber, -MOVE_READ_DISTANCE, UNLOAD_SPEED);  // Move backward by 1mm at speed 10mm/s
  }

  // retract this amount past the switch
  moveFeederAndSpoolMotor(slotNumber, -SELECTOR_OFFSET_BEFORE, UNLOAD_SPEED);
  // moveFeederMotor(slotNumber, -SELECTOR_OFFSET_BEFORE, UNLOAD_SPEED);
  // moveSpoolMotor(slotNumber, -SELECTOR_OFFSET_BEFORE, UNLOAD_SPEED);


  moveSpoolMotor(slotNumber, -SPOOL_EXTRA_REWIND, UNLOAD_SPEED);

  // Turn motor off
  digitalWrite(slots[slotNumber-1].feederEnablePin, MOTOR_OFF);
  digitalWrite(slots[slotNumber-1].spoolEnablePin, MOTOR_OFF);


  Serial.println("[SLOT-" + String(slotNumber) + "] Unloaded!");
}


void loadSlot(uint8_t slotNumber) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

  uint8_t currentLoadedSlot = Selector::getLoadedInput();  // Check if any slot is currently loaded
  if (currentLoadedSlot == 0) {
    Serial.println("[SLOT-" + String(slotNumber) + "] Loading...");

    // Load by moving forward until loaded
    while (!Selector::inputHasFilament(slotNumber)) {
      moveFeederMotor(slotNumber, MOVE_READ_DISTANCE, LOAD_SPEED);  // Move by 1mm at LOAD_SPEED
      // moveSpoolMotor(slotNumber, MOVE_READ_DISTANCE, LOAD_SPEED);  // Move by 1mm at LOAD_SPEED
    }

    // Load by moving forward until loaded
    while (!Selector::isLoaded()) {
      moveFeederMotor(slotNumber, MOVE_READ_DISTANCE, LOAD_SPEED);  // Move by 1mm at LOAD_SPEED
      // moveSpoolMotor(slotNumber, MOVE_READ_DISTANCE, LOAD_SPEED);  // Move by 1mm at LOAD_SPEED
    }
    
    // Move this amount past the selector output
    moveFeederMotor(slotNumber, SELECTOR_OFFSET_AFTER, LOAD_SPEED);
    // moveSpoolMotor(slotNumber, SELECTOR_OFFSET_AFTER, LOAD_SPEED);
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

  uint8_t currentLoadedSlot = Selector::getLoadedInput();  // Check if any slot is currently loaded

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
    uint8_t currentLoadedSlot = Selector::getLoadedInput();

    // feed until it finds resistance in the buffed (unclick switch)
    Serial.print("[Buffer] Feeding");
    while (digitalRead(BUFFER_PIN) == BUFFER_EMPTY_LEVEL) {
      moveFeederMotor(currentLoadedSlot, MOVE_READ_DISTANCE, FEED_SPEED);  // Move forward by 1mm at FEED_SPEED
      Serial.print(".");
    }
    Serial.println("");

    // compress the spring by fixed amount
    Serial.println("[Buffer] Preloading...");
    moveFeederMotor(currentLoadedSlot, BUFFER_PRELOAD_LENGH, PRELOAD_SPEED);

    Serial.println("[Buffer] Idle");
    while (digitalRead(BUFFER_PIN) == !BUFFER_EMPTY_LEVEL); // wait for buffer to be empty
  }
}



void setup() {
  Serial.begin(115200);
  Serial.println("[TMS] BOOTING...");

  // Configure Slots
  Serial.println("Initializing " + String(MAX_SLOTS) + " slots... ");
  for (uint8_t i = 0; i < MAX_SLOTS; i++) {
    pinMode(slots[i].filamentSensorPin, INPUT_PULLUP);
    
    // Feeder Motor
    pinMode(slots[i].feederEnablePin, OUTPUT);
    pinMode(slots[i].feederDirPin, OUTPUT);
    pinMode(slots[i].feederStepPin, OUTPUT);
    digitalWrite(slots[i].feederEnablePin, !MOTOR_ON); // Turn motor off by default

    // Spool Motor
    pinMode(slots[i].spoolEnablePin, OUTPUT);
    pinMode(slots[i].spoolDirPin, OUTPUT);
    pinMode(slots[i].spoolStepPin, OUTPUT);
    digitalWrite(slots[i].spoolEnablePin, !MOTOR_ON); // Turn motor off by default

    Serial.println("[SLOT-" + String(i+1) + "] " + "initialized!");
  }

  // Configure Selector
  Serial.println("Initializing Selector... ");
  for (uint8_t i = 0; i < MAX_SLOTS; i++) {
    pinMode(slots[i].selectorInputPin, INPUT_PULLUP);
  }
  pinMode(SELECTOR_END_PIN, INPUT_PULLUP);
  Serial.println("[SELECTOR] initialized!");

  // Configure Buffer
  Serial.println("Initializing Buffer... ");
  pinMode(BUFFER_PIN, INPUT_PULLUP);
  Serial.println("[BUFFER] initialized!");


  // ---- RUN SELF CHECKS ----
  Serial.println("\n[TMS] Running self check...");

  // Check which slot is currently loaded, 0 if none
  int selectorLoadedCount = 0;
  for (uint8_t i = 1; i <= MAX_SLOTS; i++) {
    if (Selector::inputHasFilament(i)) {
      selectorLoadedCount++;
      Serial.println("[SELECTOR] Input " + String(i) + " has filament");
    }
  }

  if (selectorLoadedCount==0) {
    Serial.println("[SELECTOR] All inputs are empty");
  }
  else if (selectorLoadedCount>1) {
    Serial.println("[SELECTOR] ERROR! More then 1 input is has filament!");
  }
  
  bool isSelectorLoaded = Selector::isLoaded();
  if (isSelectorLoaded) Serial.println("[SELECTOR] Output has filament");
  else Serial.println("[SELECTOR] Output is empty");
  
  if (selectorLoadedCount==0 && isSelectorLoaded) {
    Serial.println("[SELECTOR] ERROR! All the inputs are empty but the Selector output has filament!");
  }
  else Serial.println("[SELECTOR] Self check: PASS!");

  bool isBufferEmpty = digitalRead(BUFFER_PIN) == BUFFER_EMPTY_LEVEL;
  if (!isBufferEmpty && selectorLoadedCount==0 && isSelectorLoaded) {
    Serial.println("ERROR! Buffer is jammed!");
  }
  else if (!isBufferEmpty && selectorLoadedCount==0) {
    Serial.println("ERROR! Selector output has no filament but Buffer says there is!");
  }
  else Serial.println("[BUFFER] Self check: PASS!");


  Serial.println("\nTMS Ready!\n");
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

      moveFeederMotor(slotNumber, length, speed);
      Serial.println("End.");
    }


    // ROLL <slot> <length> <speed>
    // Move a motor slot by a certain amount
    else if (command.startsWith("ROLL ")) {
      int firstSpace = command.indexOf(' ', 5);
      int secondSpace = command.indexOf(' ', firstSpace + 1);
      // Extracting slot number, length, and speed
      int slotNumber = command.substring(5, firstSpace).toInt();
      float length = command.substring(firstSpace + 1, secondSpace).toFloat();
      float speed = command.substring(secondSpace + 1).toFloat();
      if (slotNumber < 1 || slotNumber > MAX_SLOTS) return Serial.println("Invalid slot");

      Serial.println("Moving Spool " + String(slotNumber) + " by " + String(length) + "mm at " + String(speed) + "mm/s");

      moveSpoolMotor(slotNumber, length, speed);
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
      Serial.println("Invalid command!");
    }
  }
}
