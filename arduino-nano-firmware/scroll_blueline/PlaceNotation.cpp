#include "Arduino.h"
#include "PlaceNotation.h"
#include "Util.h"

////////////////////////////////////////////////////
//    NEW CODE for new PN per row approach

// update bell pos like '1' using a notate like '14' or 'x'.

void updateBellPosChar(char *p, char *n) {
  char *s = n;
  while (*n < *p && *n) n++; // step over notate until find a place >= to our bell position
  if (*n == *p) return; // if making a place, do nothing
  *p += ((n - s) ^ (*p - '1')) & 1 ? -1 : 1; // step left or right based on parity
}

// this can be written more efficiently, I'm sure
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
  return resultCount;
}

void apply_place_notation(char* row, const char* notation) {
  static bool workingIsPlace[10];

  // PRINTFLN("================");
  // PRINT_VAR("Row: ", row);
  // PRINT_VAR("Notation: ", notation);

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
  // PRINT_VAR("End of PN processing, got row = ", row);
}

