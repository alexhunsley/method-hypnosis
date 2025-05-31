#include <Arduino.h>
#include "util.h"

// Memory checking 

extern int __heap_start, *__brkval;

int freeMemory() {
  int v;
  return (int)&v - (__brkval == 0 ? (int)&__heap_start : (int)__brkval);
}

// Voltage

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

// strings

char* copy_substring(const char* src, int start, int len) {
  // Allocate memory (+1 for null terminator)
  char* result = (char*) malloc(len + 1);
  if (result == NULL) return NULL;  // check for allocation failure

  // Copy substring
  strncpy(result, src + start, len);
  result[len] = '\0';  // null-terminate

  return result;
}
