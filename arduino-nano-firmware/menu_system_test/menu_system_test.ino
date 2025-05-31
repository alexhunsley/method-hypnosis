#include "MenuSystem.h"
// reminder: the cpp file must be added to this sketch (Sketch -> Add file...) for import to work.
// (the file doesn't need moving, just point at it in the sibling folder)

#include <MD_MAX72xx.h>

#define MAX_DEVICES 4
#define CS_PIN 10

MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::FC16_HW, CS_PIN, MAX_DEVICES);

void setup() {
  // make LED panels low intensity, if attached
  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 1);
  mx.clear();

  mx.setPoint(0, 0, true);
  mx.setPoint(1, 0, true);

  ///////////////////////
  // menu
  Serial.begin(9600);         // Start the serial port at 9600 baud
  Serial.println("Setup started X");  // Print a line to Serial Monitor

  start_menu();
}

void loop() {
  loop_menu();
}
