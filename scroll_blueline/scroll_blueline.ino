#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "MenuSystem.h"

extern int __heap_start, *__brkval;
int freeMemory() {
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

#define DBG_MEM Serial.print(F("Free mem: ")); Serial.println(freeMemory())

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
    Serial.print(value);                 \
    Serial.println(F(label2));               \
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

#define TARGET_BELL '7'
#define MAX_DEVICES 1

#define DATA_PIN 11
#define CLK_PIN 13
#define CS_PIN 10

// note: FC16_HW is the common 4-in-1 -- for single display, need to use GENERIC_HW here!
MD_MAX72XX mx = MD_MAX72XX(MD_MAX72XX::GENERIC_HW, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);


#define MAX_ROW_LENGTH 11  // Max 10 bells + null terminator
#define MAX_METHOD_TITLE_LENGTH 20
// enough for bristol if using ',' to reverse
#define MAX_METHOD_PLACE_NOTATION_LENGTH 40
// place notation array
#define MAX_TOKENS 35
// can handle max 4 char notate like 1256 (the last char is for
// null temrinator; could get rid of need for that eventually with helper func)
#define MAX_TOKEN_LENGTH 5

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
                      // {"Bristol", "x58x14.58x58.36.14x14.58x14x18", 8},
                      // {"Bristol", "1234x18x,12", 8},
                      {"Double Norwich", "x14x36x58x18,18", 8},
                      {"Stedman", "3.1.7.3.1.3,1", 7}
                    };

int selectedMethodIdx = 2;
int selectedMethodPNCount = 0;
int selectedMethodTargetBell = 2;
int frame = 0;

bool isHalted = false;

void halt() {
  isHalted = true;
}

void setup() {
  Serial.begin(9600);
  PRINT("Serial started...");
  reportVCC();

  start_menu();

  mx.begin();
  mx.clear();
  mx.control(MD_MAX72XX::INTENSITY, 1);
}

int sleep_time = 150;
int pause_leadend_counter = 0;
int leadend_pause = 3000;
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

char* copy_substring(const char* src, int start, int len) {
  // Allocate memory (+1 for null terminator)
  char* result = (char*) malloc(len + 1);
  if (result == NULL) return NULL;  // check for allocation failure

  // Copy substring
  strncpy(result, src + start, len);
  result[len] = '\0';  // null-terminate

  return result;
}

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
    PRINTF("INIT METHOD!!!!!!!!!!!!!!!!!!!");
    memcpy(change, rounds, methods[selectedMethodIdx].stage);
    change[methods[selectedMethodIdx].stage] = '\0';
    selectedMethodPNCount = parse_place_notation_sequence(methods[selectedMethodIdx].placeNotation, expandedPN);
    selectedMethodTargetBell = '0' + methods[selectedMethodIdx].stage;
    PRINT_VAR("resultCount B : ", selectedMethodPNCount);
  }

  // debug: show bitmap as in memory
  // for (uint8_t col = 0; col < mx.getColumnCount(); col++) {
  //   byte b = mx.getColumn(col);
  //   PRINT_VAR("Col ", col);
  //   PRINTF(": ");
  //   Serial.println(b, BIN);
  // }

  if (pause_leadend_counter > 0) {
    pause_leadend_counter--;
  }
  else {
    // scroll display
    // const char* pos = strchr(change, TARGET_BELL);
    const char* pos = strchr(change, selectedMethodTargetBell);


    int plotPos = (pos != NULL) ? (pos - change) : 0;
    mx.transform(MD_MAX72XX::TSU);  // Scroll up
    mx.setPoint(7, 7 - plotPos, true);
    mx.update();

    PRINT_VAR("Not in pause, loop_count++ to ", loop_count);

    // if ((loop_count % selectedMethodPNCount) == selectedMethodPNCount - 1) {
    // // if ((loop_count % selectedMethodPNCount) == 0) {
    //   pause_leadend_counter = leadend_pause / sleep_time;
    // }
    // else {
      int pnIndex = loop_count % selectedMethodPNCount;
      // `change` is modified in-place
      apply_place_notation(change, expandedPN[pnIndex]);
      loop_count++;
    // }
  }
  delay(sleep_time);
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

  // PRINTF("DEBUG placeNotation: ");
  // PRINTLN(placeNotation);

  for (unsigned int i = 0; placeNotation[i] != '\0'; i++) {
    char c = placeNotation[i];

    if (c == ',') {
      // PRINTFLN("ALAL Found ,");

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
      // PRINTFLN("ALAL Got . or x");

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
      // PRINT_VAR("ALAL Append simple notate char: ", c);
      // PRINT_VAR("ALAL  which has len: ", currentLen);
    }
  }

  // PRINT_VAR("ALAL FINAL current: ", current);
  // PRINT_VAR(" len = ", currentLen);

  if (currentLen > 0) {
    current[currentLen] = '\0';
    if (forwardCount < MAX_TOKENS) {
      strcpy(forward[forwardCount++], current);
    }
  }

  for (int j = 0; j < forwardCount; j++) {
    // PRINT_VAR("ALAL append forward to notates arr: ", forward[j]);

    if (resultCount < MAX_TOKENS) {
      strcpy(placeNotates[resultCount++], forward[j]);
    }
  }

  // PRINT_VAR("PN array count: ", resultCount);
  //
  // for (int i = 0; i < resultCount; i++) {
  //   PRINTLN(placeNotates[i]);
  // }

  PRINT_VAR("resultCount = ", resultCount);
  return resultCount;
}

void apply_place_notation(char* row, const char* notation) {
  static bool workingIsPlace[10];

  PRINTFLN("================");
  PRINT_VAR("Row: ", row);
  PRINT_VAR("Notation: ", notation);

  int len = strlen(row);
  memset(workingIsPlace, 0, sizeof(workingIsPlace));

  for (int i = 0; notation[i] != '\0'; i++) {
    char c = notation[i];
    if (c >= '1' && c <= '9') {
      workingIsPlace[c - '1'] = true;
    } else if (c == '0') {
      workingIsPlace[9] = true;
    }
  }

  int i = 0;
  while (i < len) {
    if (workingIsPlace[i]) {
      i++;
      continue;
    }
    char temp = row[i];
    row[i] = row[i + 1];
    row[i + 1] = temp;
    i += 2;
  }
  row[i] = '\0';
  PRINT_VAR("End of PN processing, got row = ", row);
}

