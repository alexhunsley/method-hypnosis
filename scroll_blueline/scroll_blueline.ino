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

#define MAX_DEVICES 1
// #define MAX_DEVICES 1

#define DATA_PIN 11
#define CLK_PIN 13
#define CS_PIN 10

MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::FC16_HW, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
// MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::FC16_HW, CS_PIN, MAX_DEVICES);


// bristol8 (positions of tenor in each change)
// const uint8_t y_points[] = { 7, 6, 5, 4, 5, 6, 7, 7, 6, 7, 6, 7, 7, 6, 7, 6, 5, 4, 5, 4, 4, 5, 4, 5, 5, 6, 7, 7, 6, 7, 7, 6, 5, 4, 4, 5, 4, 4, 5, 6, 7, 6, 7, 6, 5, 4, 5, 4, 3, 2, 1, 0, 1, 2, 3, 3, 4, 4, 5, 6, 7, 6, 5, 4, 3, 2, 3, 2, 1, 0, 1, 0, 1, 2, 3, 3, 2, 3, 3, 2, 1, 0, 0, 1, 0, 0, 1, 2, 2, 3, 2, 3, 3, 2, 3, 2, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 2, 3, 2, 1, 0, 0, 1, 2, 3, 2, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 2, 3, 2, 3, 3, 2, 3, 2, 2, 1, 0, 0, 1, 0, 0, 1, 2, 3, 3, 2, 3, 3, 2, 1, 0, 1, 0, 1, 2, 3, 2, 3, 4, 5, 6, 7, 6, 5, 4, 4, 3, 3, 2, 1, 0, 1, 2, 3, 4, 5, 4, 5, 6, 7, 6, 7, 6, 5, 4, 4, 5, 4, 4, 5, 6, 7, 7, 6, 7, 7, 6, 5, 5, 4, 5, 4, 4, 5, 4, 5, 6, 7, 6, 7, 7, 6, 7, 6, 7, 7, 6, 5, 4, 5, 6, 7 };
// little test pattern:
const uint8_t y_points[] = { 0, 1, 0, 1, 2, 0, 1, 2, 3, 0, 1, 2, 3, 4 };

int frame = 0;

void setup() {
  Serial.begin(9600);
  Serial.println("Serial started...");

  // adding ths breaks the LED scroller (stays dark)
  start_menu();

  mx.begin();
  mx.clear();
  mx.control(MD_MAX72XX::INTENSITY, 1);
  // mx.setPoint(8, 0, true); 
}

size_t y_points_len = sizeof(y_points) / sizeof(y_points[0]);
int sleep_time = 120;
int pause_leadend_counter = 0;
int leadend_pause = 100;
int method_part = 0;
bool invert = false;

void setBrightness(int b) {
  mx.control(MD_MAX72XX::INTENSITY, b);
}

void setSpeed(int s) {
  sleep_time = s;
}

void setAll() {
  // TODO put back later
  //   for (uint8_t dev = 0; dev < MAX_DEVICES; dev++) {
  //     for (uint8_t row = 0; row < 8; row++) {
  //       for (uint8_t col = 0; col < 8; col++) {
  //         mx.setPoint(row, col + (dev * 8), true);
  //       }
  //     }
  // }
}

void loop() {
  loop_menu();

  // if (frame == 0 && invert) {
  //   setAll();
  // }

  // // mx.setPoint(0, 0, true); 
  // mx.setPoint(frame % 8, frame / 8, !invert); 

  // mx.update();
  // frame = (frame + 1) % 64;
  // if (frame == 0) {
  //   invert = !invert;
  //   if (!invert) {
  //       mx.clear();
  //   }
  // }
  // delay(sleep_time);

  
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