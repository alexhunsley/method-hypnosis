#ifndef UTIL_H
#define UTIL_H

#include "util.h"

// Console logging

#define DBG_MEM Serial.print(F("Free mem: ")); Serial.println(freeMemory())

#define ARRAY_LEN(x) (sizeof(x) / sizeof((x)[0]))

// logging helpers, including F variants to put strings in flash not RAM
// #define PRINTF(str) Serial.print(F(str))
// #define PRINTFLN(str) Serial.println(F(str))
// #define PRINT(x) Serial.print(x)
// #define PRINTLN(x) Serial.println(x)

// #define PRINT_VAR(label, value)  \
//   do {                           \
//     Serial.print(F(label));      \
//     Serial.println(value);       \
//   } while (0)

// #define PRINT_VAR2(label, value, label2)  \
//   do {                                    \
//     Serial.print(F(label));               \
//     Serial.print(value);                  \
//     Serial.println(F(label2));            \
//   } while (0)

// enable these instead of the usual to disable logging and save memory
#define PRINTF(str)
#define PRINTFLN(str)
#define PRINT(x)
#define PRINTLN(x)
#define PRINT_VAR(label, value)
#define PRINT_VAR2(label, value, label2)

// inlines must go in .h

// Voltage

inline long readVCC() {
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);  // Select 1.1V (VBG) input
  delay(2); // Let voltage settle
  ADCSRA |= _BV(ADSC);  // Start conversion
  while (bit_is_set(ADCSRA, ADSC));

  int result = ADC;
  long vcc = 1125300L / result;  // 1.1V * 1023 * 1000
  return vcc;  // in millivolts
}

inline void reportVCC() {
  PRINT_VAR2("Vcc = ", readVCC(), " mV");
}
// Memory checking 

extern int __heap_start, *__brkval;

inline int freeMemory() {
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

// strings

inline char* copy_substring(const char* src, int start, int len) {
  // Allocate memory (+1 for null terminator)
  char* result = (char*) malloc(len + 1);
  if (result == NULL) return NULL;  // check for allocation failure

  // Copy substring
  strncpy(result, src + start, len);
  result[len] = '\0';  // null-terminate

  return result;
}

#endif
