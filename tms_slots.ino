
#define LOAD_SPEED 100

#define MOTOR_ON LOW

// Define a struct to hold slot information
struct Slot {
  uint8_t hubSwitchPin;

  uint8_t enablePin;
  uint8_t dirPin;
  uint8_t stepPin;

  bool feedDir;  // direction which the motor feeds LOW/HIGH
  float stepsPerMM;
};

#define MAX_SLOTS 2  // Define the maximum number of slots
Slot slots[MAX_SLOTS] = {
  {
    6,        // hubSwitchPin
    7,        // enablePin
    9,        // dirPin
    8,        // stepPin
    LOW,      // feedDir direction which the motor feeds LOW/HIGH
    100 / 2,  // stepsPerMM
  },
  {
    5,        // hubSwitchPin
    12,       // enablePin
    10,       // dirPin
    11,       // stepPin
    LOW,      // feedDir direction which the motor feeds LOW/HIGH
    100 / 2,  // stepsPerMM
  }
};


const int bufferPin = 4;

#define PRESSED LOW
#define UNPRESSED !PRESSED

// Function to check if a slot is loaded
bool isSlotLoaded(int slotNumber) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) return false;
  // Read the hub switch pin state
  int state = digitalRead(slots[slotNumber - 1].hubSwitchPin);
  return (state == PRESSED);
}

int loadedSlot() {
  for (int i = 1; i <= MAX_SLOTS; i++) {
    if (isSlotLoaded(i)) {
      return i;  // Return the number of the loaded slot
    }
  }
  return 0;  // Return 0 if no slots are loaded
}

// Function to move a motor associated with a given slot
void moveMotorBySlot(int slotNumber, float moveDistance, float speed) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) {
    Serial.println("Invalid slot number");
    return;
  }

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
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) {
    Serial.println("Invalid slot number");
    return;
  }

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
    moveMotorBySlot(slotNumber, -1, LOAD_SPEED);  // Move backward by 1mm at speed 10mm/s
  }
  Serial.println("Unload complete");
}


void loadSlot(int slotNumber) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) {
    Serial.println("Invalid slot number");
    return;
  }

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
    moveMotorBySlot(currentLoadedSlot, 1, LOAD_SPEED);  // Move backward by 1mm at speed 10mm/s
  }
  Serial.println("Load complete");
}




void filamentSwap(int slotNumber) {
  if (slotNumber < 1 || slotNumber > MAX_SLOTS) {
    Serial.println("Invalid slot number");
    return;
  }

  int currentLoadedSlot = loadedSlot();  // Check if any slot is currently loaded

  // Unload the current loaded slot if one is loaded
  if (currentLoadedSlot == -1) {
    Serial.println("No filament currently loaded!");
  }
  if (currentLoadedSlot == slotNumber) {
    Serial.println("Slot " + String(currentLoadedSlot) + " is already loaded!");
  } else if (currentLoadedSlot != -1 && currentLoadedSlot != slotNumber) {
    Serial.println("Unloading slot " + String(currentLoadedSlot) + "...");

    // Unload by moving backward until not loaded
    while (isSlotLoaded(currentLoadedSlot)) {
      moveMotorBySlot(currentLoadedSlot, -1, LOAD_SPEED);  // Move backward by 1mm at speed 10mm/s
    }

    Serial.println("Unload complete");
  }

  // Load the specified slot
  Serial.println("loading slot " + String(slotNumber) + "...");

  // Move forward until it reaches the loaded position
  while (!isSlotLoaded(slotNumber)) {
    moveMotorBySlot(slotNumber, 1, LOAD_SPEED);  // Move forward by 1mm at speed 10mm/s
  }

  // Load 100mm of filament
  moveMotorBySlot(slotNumber, 100, LOAD_SPEED);  // Load 100mm at speed 10mm/s

  Serial.println("Load complete");
}


void feed() {

    while(true) {
    int currentLoadedSlot = loadedSlot();

      // feed until it finds resistance in the buffed (unclick switch)
      Serial.print("Feeding");
      while (digitalRead(bufferPin)==PRESSED) {
        moveMotorBySlot(currentLoadedSlot, 1, LOAD_SPEED);  // Move forward by 1mm at speed 10mm/s
        Serial.print(".");
      }
      Serial.println("");

      // compress the spring by 10mm
      Serial.println("Preloading...");
      moveMotorBySlot(currentLoadedSlot, 10, LOAD_SPEED); 

      Serial.println("Idle");
      while (digitalRead(bufferPin)==UNPRESSED);

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

    Serial.println("[Slot-" + String(i) + "] " + "initialized!");
  }


  Serial.println("Initializing Buffer... ");
  pinMode(bufferPin, INPUT_PULLUP);
  Serial.println("[Buffer] initialized!");
}
void loop() {
  if (Serial.available() > 0) {
    // Read the incoming command
    String command = Serial.readStringUntil('\n');

    // Convert the command to uppercase for case-insensitive comparison
    command.toUpperCase();

    // MOVE <SLOT> <LENGTH> <SPEED>
    if (command.startsWith("MOVE ")) {
      // Splitting the command string
      int firstSpace = command.indexOf(' ', 5);
      int secondSpace = command.indexOf(' ', firstSpace + 1);

      // Extracting slot number, length, and speed
      int slotNumber = command.substring(5, firstSpace).toInt();
      float length = command.substring(firstSpace + 1, secondSpace).toFloat();
      float speed = command.substring(secondSpace + 1).toFloat();

      // Validating and executing the command
      if (slotNumber >= 1 && slotNumber <= MAX_SLOTS) {
        Serial.print("Moving Feeder ");
        Serial.print(slotNumber);
        Serial.print(" with length ");
        Serial.print(length);
        Serial.print("mm at speed ");
        Serial.print(speed);
        Serial.println("mm/s");

        moveMotorBySlot(slotNumber, length, speed);
        Serial.println("End.");
      } else {
        Serial.println("Invalid slot number");
      }
    }

    if (command.startsWith("SWAP ")) {
      // Splitting the command string
      int firstSpace = command.indexOf(' ', 5);

      // Extracting slot number
      int slotNumber = command.substring(5, firstSpace).toInt();

      // Validating and executing the command
      if (slotNumber >= 1 && slotNumber <= MAX_SLOTS) {
        Serial.print("Loading ");
        Serial.println(slotNumber);

        filamentSwap(slotNumber);
        Serial.println("End.");
      } else {
        Serial.println("Invalid slot number");
      }
    }

    if (command.startsWith("LOAD ")) {
      // Splitting the command string
      int firstSpace = command.indexOf(' ', 5);

      // Extracting slot number
      int slotNumber = command.substring(5, firstSpace).toInt();

      // Validating and executing the command
      if (slotNumber >= 1 && slotNumber <= MAX_SLOTS) {
        Serial.print("Loading ");
        Serial.println(slotNumber);

        loadSlot(slotNumber);
        Serial.println("End.");
      } else {
        Serial.println("Invalid slot number");
      }
    }

    if (command.startsWith("UNLOAD ")) {
      // Splitting the command string
      int firstSpace = command.indexOf(' ', 7);

      // Extracting slot number
      int slotNumber = command.substring(7, firstSpace).toInt();

      // Validating and executing the command
      if (slotNumber >= 1 && slotNumber <= MAX_SLOTS) {
        Serial.print("Loading ");
        Serial.println(slotNumber);

        unloadSlot(slotNumber);
        Serial.println("End.");
      } else {
        Serial.println("Invalid slot number");
      }
    }

    
    if (command.startsWith("FEED")) {
      feed();
    }

    // ... Handle other commands ...
    else {
      Serial.println("Invalid command");
    }
  }
}
