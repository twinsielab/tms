// Define a struct to hold slot information
struct Slot {
  uint8_t selectorInputPin; // selectorInputPin (filament detector in the selector slot)

  uint8_t feederEnablePin; // feederEnablePin  (driver enable pin)
  uint8_t feederDirPin; // feederDirPin (driver direction pin)
  uint8_t feederStepPin; // feederStepPin  (driver step pin)

  bool feedDir; // feedDir  (direction which the motor feeds LOW/HIGH)
  float feederStepsPerMM; // feederStepsPerMM


  uint8_t spoolEnablePin; // spoolEnablePin  (driver enable pin)
  uint8_t spoolDirPin; // spoolDirPin (driver direction pin)
  uint8_t spoolStepPin; // spoolStepPin  (driver step pin)

  bool spoolDir; // feedDir  (direction which the motor feeds LOW/HIGH)
  float spoolStepsPerMM; // spoolStepsPerMM
  
};


