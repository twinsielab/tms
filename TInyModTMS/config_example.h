#define MAX_SLOTS 2 // Define the maximum number of slots

// Per Slot Settings
Slot slots[MAX_SLOTS] = {
  // SLOT-1
  {
     0,               // filamentSensorPin   filament sensor pin on the the slot
     5,               // selectorInputPin    filament detector pin in the selector
     6,               // feederEnablePin     feeder driver enable pin
     7,               // feederDirPin        feeder driver direction pin
     8,               // feederStepPin       feeder driver step pin
     9,               // spoolEnablePin      spool driver enable pin
    10,               // spoolDirPin         spool driver direction pin
    11,               // spoolStepPin        spool driver step pin
    LOW,              // feedDir             direction which the feeder motor feeds ↓
    LOW,              // spooldDir           direction which the spool motor unwinds⤵︎
    100,              // feederStepsPerMM
    (200 * 16) / 70,  // spoolStepsPerMM
  },
};

// Global Settings
#define LOAD_SPEED 400.0             // speed (mm/s) for the loading filament sequence 
#define UNLOAD_SPEED 400.0           // speed (mm/s) for the unloading filament sequence
#define SPOOL_EXTRA_REWIND 150.0     // rewind the spool by an extra distance (mm) after unloading
#define MOTOR_ON LOW                 // state which the motor is turned on by the driver
#define MOVE_READ_DISTANCE 1.0       // distance (mm) the filament moves between every sensor read

// Selector Settings
#define SELECTOR_END_PIN 3           // filament detector pin at the end of selector
#define SELECTOR_DETECT_LEVEL LOW    // level when the Selector has filament
#define SELECTOR_OFFSET_BEFORE 30.0  // how much to retract before the sensor
#define SELECTOR_OFFSET_AFTER  95.0  // how much to feed after the sensor

// Buffer Settings
#define BUFFER_PIN 4                 // pressure detection pin on the buffer
#define BUFFER_EMPTY_LEVEL LOW       // level when the Buffer is empty
#define FEED_SPEED 200.0             // speed (mm/s) which filament will be feed when theres no pressure on the buffer
#define PRELOAD_SPEED 100.0          // speed (mm/s) which the buffer spring will be compressed
#define BUFFER_PRELOAD_LENGH 15.0    // how much preload the buffer spring will be compressed
