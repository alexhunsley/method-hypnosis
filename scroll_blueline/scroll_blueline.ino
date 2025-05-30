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

#define DATA_PIN 11
#define CLK_PIN 13
#define CS_PIN 10

MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::FC16_HW, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
// MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::FC16_HW, CS_PIN, MAX_DEVICES);

struct Method {
  const char* title;
  const char* placeNotation;
};

Method methods[] = {{"Bristol", "x58x14.58x58.36.14x14.58x14x18,18"},
                    {"Double Norwich", "x14x36x58x18,18"}};


const int MAX_TOKENS = 64; // expanded PN chars
int selectedMethodIdx = 1;
int selectedMethodPNCount = 0;
// String expandedPN = "";

// String output[MAX_TOKENS];

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

// size_t y_points_len = sizeof(y_points) / sizeof(y_points[0]);
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

String expandedPN[MAX_TOKENS];

void loop() {
  loop_menu();

  if (selectedMethodPNCount == 0) {
    // RECENT I removed & from the below.
    selectedMethodPNCount = parse_place_notation_sequence(methods[selectedMethodIdx].placeNotation, expandedPN);
    Serial.println("Processing the PN....");
    // Serial.println(expandedPN);
  }



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
String change = "12345678";

  // mx.clear();


  // mx.update();
  // delay(sleep_time * 3);

  // with cable going into LEDs at top:
  // x goes right, y goes down.
  // int x = 1;
  // int y = 0;
  // mx.setPoint(x, y, true);

  // TODO scroll the display!

  // for (uint8_t y = 0; y < MAX_DEVICES * 8; y++) {
  //   // int y = methodPos - col;

  //   // if (y >= 0 && y < (MAX_DEVICES * 8)) {
  //     uint8_t x = y_points[(y + methodPos) % y_points_len];
  //     if (x < 8) {
  //       mx.setPoint(x, y, true);
  //     }
  //   // }
  // // }
  // }

// TODO put back later
  // mx.transform(MD_MAX72XX::TSL);  // Scroll up

  // int plotPos = change.indexOf("8");
  int plotPos = 4;
  mx.setPoint(plotPos, 0, true);

  change = apply_place_notation(change, String(expandedPN[methodPos]));
  Serial.println(change);

  // mx.setPoint(1, 0, true);

  methodPos = (methodPos + 1) % selectedMethodPNCount;

  mx.update();
  delay(sleep_time);

  // if (pause_leadend_counter > 0) {
  //   pause_leadend_counter--;
  // }
  // else {
  //   methodPos = (methodPos + 1) % y_points_len; // - MAX_DEVICES * 8);
  //   if ((methodPos % 32) == 1) {
  //       pause_leadend_counter = leadend_pause / sleep_time;

  //       method_part = (method_part + 1) % 8;
  //   }
  // }
}

////////////////////////////////////////////////////
//    NEW CODE for new PN per row approach

int parse_place_notation_sequence(const String& seq, String result[]) {
  String current = "";
  String forward[MAX_TOKENS];
  int forwardCount = 0;
  int resultCount = 0;

  for (unsigned int i = 0; i < seq.length(); i++) {
    char c = seq[i];
    
    if (c == ',') {
      if (current.length() > 0) {
        forward[forwardCount++] = current;
        current = "";
      }

      // Append forward to result
      for (int j = 0; j < forwardCount; j++) {
        result[resultCount++] = forward[j];
      }

      // Append reversed forward (excluding the first item in reverse)
      for (int j = forwardCount - 2; j >= 0; j--) {
        result[resultCount++] = forward[j];
      }

      forwardCount = 0; // Clear forward
    }
    else if (c == '.' || c == 'x') {
      if (current.length() > 0) {
        forward[forwardCount++] = current;
        current = "";
      }
      if (c == 'x') {
        forward[forwardCount++] = "x";
      }
    }
    else {
      current += c;
    }
  }

  if (current.length() > 0) {
    forward[forwardCount++] = current;
  }

  // Append remaining forward to result
  for (int j = 0; j < forwardCount; j++) {
    result[resultCount++] = forward[j];
  }

  return resultCount;  // Return the number of elements stored in result[]
}

String apply_place_notation(String row, String notation) {
  int len = row.length();

  if (notation == "x") {
    // Cross: swap all adjacent pairs
    String result = "";
    for (int i = 0; i < len; i += 2) {
      if (i + 1 < len) {
        result += row[i + 1];
        result += row[i];
      } else {
        result += row[i]; // Leave last unpaired character
      }
    }
    return result;
  } else {
    // Parse places
    bool isPlace[10] = {false};  // up to 10 bells max (0-indexed)

    for (int i = 0; i < notation.length(); i++) {
      char c = notation[i];
      if (c >= '1' && c <= '9') {
        isPlace[c - '1'] = true;
      } else if (c == '0') {  // handle 10 as '0'
        isPlace[9] = true;
      }
    }

    // Copy original row to modifiable array
    char new_row[11];  // support up to 10 chars + null terminator
    row.toCharArray(new_row, 11);

    // Swap all non-place adjacent pairs
    int i = 0;
    while (i < len - 1) {
      if (isPlace[i]) {
        i++;
        continue;
      }
      // swap i and i+1
      char temp = new_row[i];
      new_row[i] = new_row[i + 1];
      new_row[i + 1] = temp;
      i += 2;
    }

    return String(new_row);
  }
}

// void setup() {
//   Serial.begin(9600);

//   String output[MAX_TOKENS];
//   int count = parse_place_notation_sequence("x58x14.58x58.36.14x14.58x14x18,18", output);

//   String row = "12345678";
//   Serial.println("Initial row: " + row);

//   for (int i = 0; i < count; i++) {
//     row = apply_place_notation(row, output[i]);
//     Serial.println("After " + output[i] + ": " + row);
//   }
// }

// void loop() {
//   // put your main code here, to run repeatedly:
//   // sleep(100);
// }


