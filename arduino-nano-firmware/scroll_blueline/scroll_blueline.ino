#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "MenuSystem.h"
#include "util.h"
#include "PlaceNotation.h"
#include "scroll_blueline.h"

// this script runs on the 4x1 LED panels from AZDelivery (it's 4 8x8 LEDs next to each other)
// Just wire it up to the 5 pins given in the instructions.
//
// some usable font defs: https://github.com/dhepper/font8x8

// If using MaxPanel:
//
// There's a dependency issue with MaxPanel lib: https://arduino.stackexchange.com/a/97015
//
// So uncheck the Arduino IDE Library manager's 'check dependencies' option to
// get MaxPanel to install.

#define TARGET_BELL '7'
#define MAX_DEVICES 1

#define DATA_PIN 11
#define CLK_PIN 13
#define CS_PIN 10

// note: FC16_HW is the common 4-in-1 -- for single display, need to use GENERIC_HW here!
MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::GENERIC_HW, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

// 11 = 10 bells plus terminator
char* change = (char*)malloc(11);
const char* rounds = "1234567890";

struct Method {
  char title[MAX_METHOD_TITLE_LENGTH];
  char placeNotation[MAX_METHOD_PLACE_NOTATION_LENGTH];
  const int stage;
};

const Method methods[] = {
                      {"Bristol", "x58x14.58x58.36.14x14.58x14x18,18", 8},
                      {"Stedman", "3.1.7.3.1.3,1", 7},
                      {"Double Norwich", "x14x36x58x18,18", 8},
                    };

int blueline_sleep_time = 75;

int selectedMethodIdx = 2;
int selectedMethodPNCount = 0;
int selectedMethodTargetBell = 2;
int frame = 0;

bool isHalted = false;

// void halt() {
//   isHalted = true;
// }

void setup() {
  Serial.begin(9600);
  PRINT("Serial started...");
  reportVCC();

  start_menu();

  mx.begin();
  mx.clear();
  mx.control(MD_MAX72XX::INTENSITY, 1);
}

int pause_leadend_counter = 0;
int leadend_pause = 1000;
int method_part = 0;
bool invert = false;

void loop() {
  static int loop_count = 0;
  static char expandedPN[MAX_TOKENS][MAX_TOKEN_LENGTH];

  if (isHalted) {
    // big sleep for you, soldier
    delay(UINT32_MAX);
    return;
  }

  DBG_MEM;
  loop_menu();

  if (selectedMethodPNCount == 0) {
    loop_count = 0;
    memcpy(change, rounds, methods[selectedMethodIdx].stage);
    change[methods[selectedMethodIdx].stage] = '\0';
    selectedMethodPNCount = parse_place_notation_sequence(methods[selectedMethodIdx].placeNotation, expandedPN);
    selectedMethodTargetBell = '0' + methods[selectedMethodIdx].stage;
    mx.clear();
  }

  if (pause_leadend_counter > 0) {
    pause_leadend_counter--;
  }
  else {
    const char* pos = strchr(change, selectedMethodTargetBell);

    int plotPos = (pos != NULL) ? (pos - change) : 0;
    mx.transform(MD_MAX72XX::TSU);  // Scroll up
    mx.setPoint(7, 7 - plotPos, true);
    mx.update();

    int pnIndex = loop_count % selectedMethodPNCount;
    apply_place_notation(change, expandedPN[pnIndex]);
    loop_count++;
  }
  delay(blueline_sleep_time);
}
