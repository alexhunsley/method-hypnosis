#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <MD_MAX72xx.h>
#include <SPI.h>
#include "MenuSystem.h"

// #define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

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

const String rounds = "1234567890";

struct Method {
  String title;
  String placeNotation;
  const int stage;
};

Method methods[] = {
                    // {"Bristol", "x58x14.58x58.36.14x14.58x14x18,18", 8},
                    // {"Bristol", "x58x14.58x58.36.14x14.58x14x18", 8},
                    {"Bristol", "1234x18x,12", 8},
                    {"Double Norwich", "x14x36x58x18,18", 8}
                   };


const int MAX_TOKENS = 24; // expanded PN chars
int selectedMethodIdx = 0;
int selectedMethodPNCount = 0;
// String expandedPN = "";

// String output[MAX_TOKENS];

int frame = 0;

void setup() {
  Serial.begin(9600);
  Serial.println(F("Serial started..."));
  reportVCC();

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
  Serial.print(F("Vcc = "));
  Serial.print(readVcc());
  Serial.println(F(" mV"));
}

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

// TODO can just omit ""? It's set later
String change = "";

// triggers method change
void setMethodIndex(int methodIndex) {
  selectedMethodIdx = methodIndex;
  // do this last! (but maybe mono-threaded, anyway)
  selectedMethodPNCount = 0;
}

void printStringArray(String stringArr[], int count) {
    for (int i = 0; i < count; i++) {
      Serial.println(stringArr[i]);
    }
}

void loop() {
  static int loop_count = 0;
  // Serial.print(">>>>>>>> loop: count = ");
  // Serial.println(loop_count);
  loop_count++;

  loop_menu();

  if (loop_count < 2) { 
    // this all looks good
    Serial.println(methods[0].placeNotation);  // Check before any manipulation
    Serial.println(methods[0].stage);  // Check before any manipulation
    Serial.println(methods[0].title);  // Check before any manipulation
    Serial.println(methods[1].placeNotation);  // Check before any manipulation

    Serial.print(F("got stage: "));
    Serial.println(methods[selectedMethodIdx].stage);

    change = rounds.substring(0, methods[selectedMethodIdx].stage);
    // change = rounds.substring(0, 8);

    Serial.print(F("made start rounds change: "));
    Serial.println(change);

    String expandedPN[MAX_TOKENS];
    // calling this seems to corrupt stuff!
    selectedMethodPNCount = parse_place_notation_sequence(methods[selectedMethodIdx].placeNotation, expandedPN);

    Serial.print(F("selectedPN count: "));
    Serial.println(selectedMethodPNCount);
    Serial.print(F("PNs: "));
    for (uint8_t i = 0; i < selectedMethodPNCount; i++) {
      Serial.println(expandedPN[i]);
    }
    Serial.print(F(" --- DONE"));

  }

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
int parse_place_notation_sequence(const String& placeNotation, String placeNotates[]) {
  String current = "";
  String forward[MAX_TOKENS];
  int forwardCount = 0;
  int resultCount = 0;

  Serial.print(F("DEBUG placeNotation: '"));
  Serial.print(placeNotation);
  Serial.println("'");

  // is ok: 
  // Serial.print(F("parse_place_notation_sequence: placeNotation = "));
  // Serial.println(placeNotation);
  // Serial.println(F("-FINI"));
  
  for (unsigned int i = 0; i < placeNotation.length(); i++) {
    
    char c = placeNotation[i];

    // is ok
    // Serial.print(F("i: "));
    // Serial.println(i);
    // Serial.print(F("c: "));
    // Serial.println(c);

    if (c == ',') {
      Serial.println(F("ALAL Found ,"));
      if (current.length() > 0) {
        forward[forwardCount++] = current;
        current = "";
      }

      // Append forward to result
      for (int j = 0; j < forwardCount; j++) {
        placeNotates[resultCount++] = forward[j];
      }

      // Append reversed forward (excluding the first item in reverse)
      for (int j = forwardCount - 2; j >= 0; j--) {
        placeNotates[resultCount++] = forward[j];
      }

      forwardCount = 0; // Clear forward
    }
    else if (c == '.' || c == 'x') {
      Serial.print(F("ALAL Got . or x"));

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
      Serial.print(F("ALAL Append simple notate char: "));
      Serial.println(c);
      Serial.print(F("ALAL  which has len: "));
      Serial.println(current.length());
    }
  }

  Serial.print(F("ALAL FINAL current: '"));
  Serial.print(current);
  Serial.print(F("' (len = "));
  Serial.print(current.length());
  Serial.println(F(")"));

  if (current.length() > 0) {
      Serial.print(F("ALAL Assign current to forward: "));
      Serial.println(current);
      forward[forwardCount++] = current;
  }

  // Append remaining forward to result
  for (int j = 0; j < forwardCount; j++) {
    Serial.print(F("ALAL append forward to notates arr: "));
    Serial.print(forward[j]);
    Serial.println();

    placeNotates[resultCount++] = forward[j];
  }

  Serial.print("PN array count: ");
  Serial.println(resultCount);
  Serial.print("PN: ");
  printStringArray(placeNotates, ARRAY_LEN(placeNotates));

  return resultCount;  // Return the number of elements stored in result[]
}

String apply_place_notation(String row, String notation) {
  Serial.print(F("Row: "));
  Serial.println(row);
  Serial.print(F("Notation: "));
  Serial.println(notation);
  
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


