#include "MenuSystem.h"
// reminder: the cpp file must be added to this sketch (Sketch -> Add file...) for import to work.
// (the file doesn't need moving, just point at it in the sibling folder)

///////////////////////
// give the LED panel low intensity
#include <MD_MAX72xx.h>
#include <SPI.h>

#define MAX_DEVICES 4  // You have 4 matrices (4 × 8x8 = 32x8)
#define CS_PIN 10       // Chip Select (LOAD) — change if needed

MD_MAX72XX ledMatrix = MD_MAX72XX(MD_MAX72XX::FC16_HW, CS_PIN, MAX_DEVICES);
///////////////////////

void setup() {
  // make LED low intensity, if attached
  ledMatrix.begin();       // or MD_MAX72XX initialization
  ledMatrix.control(MD_MAX72XX::INTENSITY, 0);  // low brightness
  ledMatrix.clear();

  // ledMatrix.setChar(0, 'H');
  // ledMatrix.setChar(1, 'i');

  ///////////////////////
  // menu
  Serial.begin(9600);         // Start the serial port at 9600 baud
  Serial.println("Setup started X");  // Print a line to Serial Monitor

  start_menu();
}

void loop() {
  loop_menu();
}
