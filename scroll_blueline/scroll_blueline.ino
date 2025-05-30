#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "MenuSystem.h"

#define DBG_MEM() Serial.print(F("Free mem: ")); Serial.println(freeMemory())

// logging helpers, including F variants to put strings in flash not RAM
#define PRINTF(str) Serial.print(F(str))
#define PRINTFLN(str) Serial.println(F(str))
#define PRINT(x) Serial.print(x)
#define PRINTLN(x) Serial.println(x)

#define PRINT_VAR(label, value)   \
  do {                            \
    Serial.print(F(label));       \
    Serial.println(value);        \
  } while (0)

#define PRINT_VAR2(label, value, label2)   \
  do {                                     \
    Serial.print(F(label));                \
    Serial.println(value);                 \
    Serial.print(F(label2));               \
  } while (0)

// enable these instead of the usual to disable logging and save memory
// #define PRINTF(str)
// #define PRINTFLN(str)
// #define PRINT(x)
// #define PRINTLN(x)
// #define PRINT_VAR(label, value)
// #define PRINT_VAR2(label, value, label2)


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

#define MAX_DEVICES 1

#define DATA_PIN 11
#define CLK_PIN 13
#define CS_PIN 10

MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::FC16_HW, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);

#define MAX_METHOD_TITLE_LENGTH 16
// enough for bristol if using ',' to reverse
#define MAX_METHOD_PLACE_NOTATION_LENGTH 40
// place notation array
#define MAX_TOKENS 32
// can handle max 4 char notate like 1256 (the last char is for
// null temrinator; could get rid of need for that eventually with helper func)
#define MAX_TOKEN_LENGTH 5

String change = "";
const String rounds = "1234567890";

struct Method {
  char title[MAX_METHOD_TITLE_LENGTH];
  char placeNotation[MAX_METHOD_PLACE_NOTATION_LENGTH];
  const int stage;
};

const Method methods[] = {
                      // {"Bristol", "x58x14.58x58.36.14x14.58x14x18,18", 8},
                      // {"Bristol", "x58x14.58x58.36.14x14.58x14x18", 8},
                      {"Bristol", "1234x18x,12", 8},
                      {"Double Norwich", "x14x36x58x18,18", 8}
                    };

int selectedMethodIdx = 0;
int selectedMethodPNCount = 0;
int frame = 0;

void setup() {
  Serial.begin(9600);
  PRINT("Serial started...");
  reportVCC();

  start_menu();

  mx.begin();
  mx.clear();
  mx.control(MD_MAX72XX::INTENSITY, 1);
}

int sleep_time = 120;
int pause_leadend_counter = 0;
int leadend_pause = 100;
int method_part = 0;
bool invert = false;

long readVcc() {
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // Select 1.1V (VBG) input
  delay(2); // Let voltage settle
  ADCSRA |= _BV(ADSC);  // Start conversion
  while (bit_is_set(ADCSRA, ADSC));

  int result = ADC;
  long vcc = 1125300L / result;  // 1.1V * 1023 * 1000
  return vcc;  // in millivolts
}

void reportVCC() {
  PRINT_VAR2("Vcc = ", readVcc(), " mV");
}

void setBrightness(int b) {
  mx.control(MD_MAX72XX::INTENSITY, b);
}

void setSpeed(int s) {
  sleep_time = s;
}

// void setAll() {
//   // TODO put back later
//     for (uint8_t dev = 0; dev < MAX_DEVICES; dev++) {
//       for (uint8_t row = 0; row < 8; row++) {
//         for (uint8_t col = 0; col < 8; col++) {
//           mx.setPoint(row, col + (dev * 8), true);
//         }
//       }
//   }
// }

// triggers method change
void setMethodIndex(int methodIndex) {
  selectedMethodIdx = methodIndex;
  // do this last! (but maybe mono-threaded, anyway)
  selectedMethodPNCount = 0;
}

void printStringArray(String stringArr[], int count) {
    for (int i = 0; i < count; i++) {
      PRINTLN(stringArr[i]);
    }
}

void loop() {
  static int loop_count = 0;
  PRINT_VAR(">>>>>>>> loop: count = ", loop_count);
  loop_count++;

  loop_menu();

  if (loop_count < 2) { 
    PRINTLN(methods[0].placeNotation);
    PRINTLN(methods[0].stage);
    PRINTLN(methods[0].title);
    PRINTLN(methods[1].placeNotation);

    PRINT_VAR("got stage: ", methods[selectedMethodIdx].stage);

    change = rounds.substring(0, methods[selectedMethodIdx].stage);
    // change = rounds.substring(0, 8);

    PRINT_VAR("made start rounds change: ", change);

    char expandedPN[MAX_TOKENS][MAX_TOKEN_LENGTH];

    // calling this seems to corrupt stuff!
    selectedMethodPNCount = parse_place_notation_sequence(methods[selectedMethodIdx].placeNotation, expandedPN);

    PRINT_VAR("selectedPN count: ", selectedMethodPNCount);
    PRINTF("PNs: ");
    for (uint8_t i = 0; i < selectedMethodPNCount; i++) {
      PRINTLN(expandedPN[i]);
    }
    PRINTFLN(" --- DONE");
  }

  // the real code

// start comment out
  // // TODO we change selectedMethodIdx and do "selectedMethodPNCount = 0" to trigger this again
  // // on user method change
  // if (selectedMethodPNCount == 0) {

  //   // RECENT I removed & from the below.
  //   Serial.println("Processing the new PN.... (should only happen at start and once per method change)");
    
  //   Serial.print("methods[selectedMethodIdx].placeNotation is: ");
  //   Serial.println(methods[selectedMethodIdx].placeNotation);
  //   Serial.print("-- FINI2");

  //   selectedMethodPNCount = parse_place_notation_sequence(methods[selectedMethodIdx].placeNotation, expandedPN);
  //   // Serial.print("... and the new pn count: ");
  //   // Serial.println(selectedMethodPNCount);

  //   // Serial.print("Expanded PN: ");
  //   // // Serial.println(expandedPN);
  //   // // printStringArray(expandedPN, ARRAY_LEN(expandedPN));
  //   // Serial.print("First array item: ");
  //   // Serial.println(expandedPN[0]);
  // }


  // // if (frame == 0 && invert) {
  // //   setAll();
  // // }

  // // // mx.setPoint(0, 0, true); 
  // // mx.setPoint(frame % 8, frame / 8, !invert); 

  // // mx.update();
  // // frame = (frame + 1) % 64;
  // // if (frame == 0) { 
  // //   invert = !invert;
  // //   if (!invert) {
  // //       mx.clear();
  // //   }
  // // }
  // // delay(sleep_time);

  
  // // static int scrollPos = MAX_DEVICES * 8;
  // static int methodPos = 0;

  // // mx.clear();


  // // mx.update();
  // // delay(sleep_time * 3);

  // // with cable going into LEDs at top:
  // // x goes right, y goes down.
  // // int x = 1;
  // // int y = 0;
  // // mx.setPoint(x, y, true);

  // // TODO scroll the display!

  // // for (uint8_t y = 0; y < MAX_DEVICES * 8; y++) {
  // //   // int y = methodPos - col;

  // //   // if (y >= 0 && y < (MAX_DEVICES * 8)) {
  // //     uint8_t x = y_points[(y + methodPos) % y_points_len];
  // //     if (x < 8) {
  // //       mx.setPoint(x, y, true);
  // //     }
  // //   // }
  // // // }
  // // }

  // mx.transform(MD_MAX72XX::TSL);  // Scroll up
  // // int plotPos = 4;
  // int plotPos = change.indexOf("8");
  // // TODO you might want plotPos on the y bit here, not x
  // mx.setPoint(plotPos, 0, true);

  // // change = apply_place_notation(change, String(expandedPN[methodPos]));
  // change = apply_place_notation(change, expandedPN[methodPos]);
  // Serial.print("Change: ");
  // Serial.println(change);

  // // mx.setPoint(1, 0, true);

  // methodPos = (methodPos + 1) % selectedMethodPNCount;

  // mx.update();
  // END comment out

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

// USE F STRINGS!!!! otherwise corruption. WTF!
// Constants for sizes
// #define MAX_TOKENS 16
// #define MAX_TOKEN_LENGTH 12

int parse_place_notation_sequence(const char* placeNotation, char placeNotates[][MAX_TOKEN_LENGTH]) {
  char current[MAX_TOKEN_LENGTH];
  int currentLen = 0;

  char forward[MAX_TOKENS][MAX_TOKEN_LENGTH];
  int forwardCount = 0;
  int resultCount = 0;

  PRINTF("DEBUG placeNotation: ");
  PRINTLN(placeNotation);

  for (unsigned int i = 0; placeNotation[i] != '\0'; i++) {
    char c = placeNotation[i];

    if (c == ',') {
      PRINTFLN("ALAL Found ,");

      if (currentLen > 0) {
        current[currentLen] = '\0';
        if (forwardCount < MAX_TOKENS) {
          strcpy(forward[forwardCount++], current);
        }
        currentLen = 0;
      }

      for (int j = 0; j < forwardCount; j++) {
        if (resultCount < MAX_TOKENS) {
          strcpy(placeNotates[resultCount++], forward[j]);
        }
      }
      for (int j = forwardCount - 2; j >= 0; j--) {
        if (resultCount < MAX_TOKENS) {
          strcpy(placeNotates[resultCount++], forward[j]);
        }
      }

      forwardCount = 0;
    }
    else if (c == '.' || c == 'x') {
      PRINTFLN("ALAL Got . or x");

      if (currentLen > 0) {
        current[currentLen] = '\0';
        if (forwardCount < MAX_TOKENS) {
          strcpy(forward[forwardCount++], current);
        }
        currentLen = 0;
      }

      if (c == 'x') {
        if (forwardCount < MAX_TOKENS) {
          strcpy(forward[forwardCount++], "x");
        }
      }
    }
    else {
      if (currentLen < MAX_TOKEN_LENGTH - 1) {
        current[currentLen++] = c;
        current[currentLen] = '\0';
      }
      PRINT_VAR("ALAL Append simple notate char: ", c);
      PRINT_VAR("ALAL  which has len: ", currentLen);
    }
  }

  PRINT_VAR("ALAL FINAL current: ", current);
  PRINT_VAR(" len = ", currentLen);

  if (currentLen > 0) {
    current[currentLen] = '\0';
    if (forwardCount < MAX_TOKENS) {
      strcpy(forward[forwardCount++], current);
    }
  }

  for (int j = 0; j < forwardCount; j++) {
    PRINT_VAR("ALAL append forward to notates arr: ", forward[j]);

    if (resultCount < MAX_TOKENS) {
      strcpy(placeNotates[resultCount++], forward[j]);
    }
  }

  PRINT_VAR("PN array count: ", resultCount);

  for (int i = 0; i < resultCount; i++) {
    PRINTLN(placeNotates[i]);
  }

  return resultCount;
}

String apply_place_notation(String row, String notation) {
  PRINT_VAR("Row: ", row);
  PRINT_VAR("Notation: ", notation);
  
  int len = row.length();

  if (notation == "x") {
    // Cross: swap all adjacent pairs
    char *result = '\0';
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
