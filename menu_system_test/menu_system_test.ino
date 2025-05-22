#include "MenuSystem.h"
// reminder: the cpp file must be added to this sketch (Sketch -> Add file...) for import to work.
// (the file doesn't need moving, just point at it in the sibling folder)

void setup() {
  Serial.begin(9600);         // Start the serial port at 9600 baud
  Serial.println("Setup started X");  // Print a line to Serial Monitor

  start_menu();
}

void loop() {
  loop_menu();
}
