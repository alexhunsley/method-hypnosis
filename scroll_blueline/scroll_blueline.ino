#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "MenuSystem.h"

//
// this script runs on the 4x1 LED panels from AZDelivery (it's 4 8x8 LEDs next to each other)
// Just wire it up to the 5 pins given in the instructions.
//
//
// some usable font defs: https://github.com/dhepper/font8x8

// If using MaxPanel:
//
// There's a dependency issue with MaxPanel lib: https://arduino.stackexchange.com/a/97015
//
// So uncheck the Arduino IDE Library manager's 'check dependencies' option to
// get MaxPanel to install.

#define MAX_DEVICES 4
#define CS_PIN 10

MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::FC16_HW, CS_PIN, MAX_DEVICES);

// bristol8 (positions of tenor in each change)
const uint8_t y_points[] = { 7, 6, 5, 4, 5, 6, 7, 7, 6, 7, 6, 7, 7, 6, 7, 6, 5, 4, 5, 4, 4, 5, 4, 5, 5, 6, 7, 7, 6, 7, 7, 6, 5, 4, 4, 5, 4, 4, 5, 6, 7, 6, 7, 6, 5, 4, 5, 4, 3, 2, 1, 0, 1, 2, 3, 3, 4, 4, 5, 6, 7, 6, 5, 4, 3, 2, 3, 2, 1, 0, 1, 0, 1, 2, 3, 3, 2, 3, 3, 2, 1, 0, 0, 1, 0, 0, 1, 2, 2, 3, 2, 3, 3, 2, 3, 2, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 2, 3, 2, 1, 0, 0, 1, 2, 3, 2, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 2, 3, 2, 3, 3, 2, 3, 2, 2, 1, 0, 0, 1, 0, 0, 1, 2, 3, 3, 2, 3, 3, 2, 1, 0, 1, 0, 1, 2, 3, 2, 3, 4, 5, 6, 7, 6, 5, 4, 4, 3, 3, 2, 1, 0, 1, 2, 3, 4, 5, 4, 5, 6, 7, 6, 7, 6, 5, 4, 4, 5, 4, 4, 5, 6, 7, 7, 6, 7, 7, 6, 5, 5, 4, 5, 4, 4, 5, 4, 5, 6, 7, 6, 7, 7, 6, 7, 6, 7, 7, 6, 5, 4, 5, 6, 7, 7 };
// little test pattern:
// const uint8_t y_points[] = { 0, 1, 0, 1, 2, 0, 1, 2, 3, 0, 1, 2, 3, 4 };

void setBrightness(int b) {
  mx.control(MD_MAX72XX::INTENSITY, b);
}

void setup() {
  Serial.begin(9600);
  Serial.println("Serial started...");
  start_menu();

  mx.begin();
  mx.control(MD_MAX72XX::INTENSITY, 5);
  mx.clear();
}

size_t y_points_len = sizeof(y_points) / sizeof(y_points[0]);
int sleep_time = 32;
int pause_leadend_counter = 0;
int leadend_pause = 1000;

int method_part = 0;

void loop() {
  loop_menu();
  
  // static int scrollPos = MAX_DEVICES * 8;
  static int methodPos = 0;

  // for (uint8_t pwm_loop = 0; pwm_loop < pwm_loop_count; pwm_loop++) {
  mx.clear();
  // mx.update();
  // delay(sleep_time * 3);

  // with cable going into LEDs at top:
  // x goes right, y goes down.
  // int x = 1;
  // int y = 0;
  // mx.setPoint(x, y, true);

  for (uint8_t y = 0; y < MAX_DEVICES * 8; y++) {
    // int y = methodPos - col;

    // if (y >= 0 && y < (MAX_DEVICES * 8)) {
      uint8_t x = y_points[(y + methodPos) % y_points_len];
      if (x < 8) {
        mx.setPoint(x, y, true);
      }
    // }
  // }
  }
  mx.update();
  delay(sleep_time);

  if (pause_leadend_counter > 0) {
    pause_leadend_counter--;
  }
  else {
    // so why does 8 appear 3 blows at back when wrap to starting point again?
    methodPos = (methodPos + 1) % y_points_len; // - MAX_DEVICES * 8);
    if ((methodPos % 32) == 1) {
        pause_leadend_counter = leadend_pause / sleep_time;

        method_part = (method_part + 1) % 8;
    }
  }
}