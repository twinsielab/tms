#define MAX_SLOTS 2 // Define the maximum number of slots

// Slot Settings
Slot slots[MAX_SLOTS] = {
  // SLOT-1
  {
    6,        // selectorSwitchPin (filament detector pin in the selector)
    7,        // enablePin    (driver enable pin)
    9,        // dirPin       (driver direction pin)
    8,        // stepPin      (driver step pin)
    LOW,      // feedDir      (direction which the motor feeds)
    100 / 2,  // stepsPerMM
  },
  // SLOT-2
  {
    5,        // selectorSwitchPin (filament detector pin in the selector)
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

// Selector Settings
#define SELECTOR_END_PIN 1            // filament detector pin at the end of selector
#define SELECTOR_DETECT_LEVEL LOW     // level when the Selector has filament
#define SELECTOR_OFFSET_BEFORE 30     // how much to retract before the sensor
#define SELECTOR_OFFSET_AFTER  95     // how much to feed after the sensor

// Buffer Settings
#define BUFFER_PIN 4
#define BUFFER_EMPTY_LEVEL LOW   // level when the Buffer is empty
#define FEED_SPEED 200           // speed (mm/s) which filament will be feed when theres no pressure on the buffer
#define PRELOAD_SPEED 100        // speed (mm/s) which the buffer spring will be compressed
#define BUFFER_PRELOAD_LENGH 15  // how much preload the buffer spring will be kept compressed
