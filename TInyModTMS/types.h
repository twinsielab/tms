// Define a struct to hold slot information
struct Slot {
  uint8_t selectorInputPin; // selectorInputPin    filament detector pin in the selector
  uint8_t feederEnablePin;  // feederEnablePin     feeder driver enable pin
  uint8_t feederDirPin;     // feederDirPin        feeder driver direction pin
  uint8_t feederStepPin;    // feederStepPin       feeder driver step pin
  uint8_t spoolEnablePin;   // spoolEnablePin      spool driver enable pin
  uint8_t spoolDirPin;      // spoolDirPin         spool driver direction pin
  uint8_t spoolStepPin;     // spoolStepPin        spool driver step pin
  bool feedDir;             // feedDir             direction which the feeder motor feeds ↓
  bool spoolDir;            // spooldDir           direction which the spool motor unwinds⤵︎
  float feederStepsPerMM;   // feederStepsPerMM
  float spoolStepsPerMM;    // spoolStepsPerMM
};


