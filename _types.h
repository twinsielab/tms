// Define a struct to hold slot information
struct Slot {
  uint8_t hubSwitchPin; // hubSwitchPin (filament detector in the hub slot)

  uint8_t enablePin; // enablePin  (driver enable pin)
  uint8_t dirPin; // dirPin (driver direction pin)
  uint8_t stepPin; // stepPin  (driver step pin)
  bool feedDir; // feedDir  (direction which the motor feeds LOW/HIGH)
  
  float stepsPerMM; // stepsPerMM
};


