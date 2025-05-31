#ifndef UTIL_H
#define UTIL_H

extern int freeMemory();
extern void reportVCC();
extern char* copy_substring(const char* src, int start, int len);

// Console logging

#define DBG_MEM Serial.print(F("Free mem: ")); Serial.println(freeMemory())

// logging helpers, including F variants to put strings in flash not RAM
// #define PRINTF(str) Serial.print(F(str))
// #define PRINTFLN(str) Serial.println(F(str))
// #define PRINT(x) Serial.print(x)
// #define PRINTLN(x) Serial.println(x)

// #define PRINT_VAR(label, value)   \
//   do {                            \
//     Serial.print(F(label));       \
//     Serial.println(value);        \
//   } while (0)

// #define PRINT_VAR2(label, value, label2)   \
//   do {                                     \
//     Serial.print(F(label));                \
//     Serial.print(value);                   \
//     Serial.println(F(label2));             \
//   } while (0)

// enable these instead of the usual to disable logging and save memory
#define PRINTF(str)
#define PRINTFLN(str)
#define PRINT(x)
#define PRINTLN(x)
#define PRINT_VAR(label, value)
#define PRINT_VAR2(label, value, label2)

#endif
