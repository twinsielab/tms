// Define a struct to hold slot information
struct Slot {
  uint8_t selectorSwitchPin; // selectorSwitchPin (filament detector in the selector slot)

  uint8_t enablePin; // enablePin  (driver enable pin)
  uint8_t dirPin; // dirPin (driver direction pin)
  uint8_t stepPin; // stepPin  (driver step pin)
  bool feedDir; // feedDir  (direction which the motor feeds LOW/HIGH)
  
  float stepsPerMM; // stepsPerMM
};


