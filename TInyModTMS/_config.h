#define MAX_SLOTS 2 // Define the maximum number of slots

// Slot Settings
Slot slots[MAX_SLOTS] = {
  // SLOT-1
  {
    6,        // hubSwitchPin (filament detector pin in the hub)
    7,        // enablePin    (driver enable pin)
    9,        // dirPin       (driver direction pin)
    8,        // stepPin      (driver step pin)
    LOW,      // feedDir      (direction which the motor feeds)
    100 / 2,  // stepsPerMM
  },
  // SLOT-2
  {
    5,        // hubSwitchPin (filament detector pin in the hub)
    12,       // enablePin    (driver enable pin)
    10,       // dirPin       (driver direction pin)
    11,       // stepPin      (driver step pin)
    LOW,      // feedDir      (direction which the motor feeds)
    100 / 2,  // stepsPerMM
  }
};

// Motor Settings
#define MOTOR_ON LOW             // state which the motor is turned on by the driver
#define LOAD_SPEED 400           // speed (mm/s) for the loading filament sequence 
#define UNLOAD_SPEED 400         // speed (mm/s) for the unloading filament sequence

// Hub Settings
#define HUB_DETECT_LEVEL LOW     // level which the hub detect filament
#define HUB_OFFSET_BEFORE 30     // how much to retract before the sensor
#define HUB_OFFSET_AFTER  95     // how much to feed after the sensor

// Buffer Settings
#define BUFFER_PIN 4
#define BUFFER_DETECT_LEVEL HIGH // level when the buffer has pressure
#define FEED_SPEED 200           // speed (mm/s) which filament will be feed when theres no pressure on the buffer
#define PRELOAD_SPEED 100        // speed (mm/s) which the buffer spring will be compressed
#define BUFFER_PRELOAD_LENGH 15  // how much preload the buffer spring will be kept compressed
