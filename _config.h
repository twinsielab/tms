
#define MOTOR_ON LOW
#define PRESSED LOW
#define LOAD_SPEED 100

#define BUFFER_PIN 4

#define MAX_SLOTS 2  // Define the maximum number of slots
Slot slots[MAX_SLOTS] = {
  // SLOT-1
  {
    6,        // hubSwitchPin (filament detector in the hub slot)
    7,        // enablePin  (driver enable pin)
    9,        // dirPin (driver direction pin)
    8,        // stepPin  (driver step pin)
    LOW,      // feedDir  (direction which the motor feeds LOW/HIGH)
    100 / 2,  // stepsPerMM
  },
  // SLOT-2
  {
    5,        // hubSwitchPin (filament detector in the hub slot)
    12,       // enablePin  (driver enable pin)
    10,       // dirPin (driver direction pin)
    11,       // stepPin  (driver step pin)
    LOW,      // feedDir  (direction which the motor feeds LOW/HIGH)
    100 / 2,  // stepsPerMM
  }
};
