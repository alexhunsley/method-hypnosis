#include <stdio.h>
#include <string.h>

#define MAX_TOKENS 32
#define MAX_TOKEN_LENGTH 5

void concatenate_tokens(char tokens[MAX_TOKENS][MAX_TOKEN_LENGTH], int tokenCount, char* result, int resultSize) {
    static char* SEPARATOR = "|";

    result[0] = '\0';

    for (int i = 0; i < tokenCount; i++) {
        if (strlen(result) + strlen(tokens[i]) + 1 < resultSize) {
            strcat(result, tokens[i]);
            strcat(result, SEPARATOR);
        } else {
            // prevent buffer overflow
            break;
        }
    }
}

void updateBellPosCharOrig(char *posChar, char* notate) {
  char origChar = *posChar;
  char* ptr = notate;
  while (*ptr < *posChar && *ptr != '\0') {
    ptr++;
  }
  if (*ptr == *posChar) { return; }

  // 0 or 1
  unsigned char parity = ((ptr - notate) % 2) ^ ((*posChar - '1') % 2);

  printf("  --- orig parity: %d\n", parity);
  // TODO the '0' on ten! '0' is before '1' of course. but represents 10 here.
  // (':' is the char after '9', hmmm)

  // removed: *ptr == 'x' && 
  // and it fixed that problem!
  // if ((*posChar - '1') % 2) {
  //   // use XOR to flip parity value (works for 0 and 1)
  //   parity ^= 1; 
  //   printf("  --- changed parity: %d\n", parity);
  // }

  // more compact ugly way:
  // parity ^= (*ptr == 'x' && ((*posChar - '1') % 2) == 1)

  // map parity 0,1 onto -1,+1
  *posChar -= parity * 2 - 1;  
  printf("pos: %c  PN: %s  updated pos: %c\n", origChar, notate, *posChar);
}

// ok this compact one works!
// void updateBellPosChar(char *posChar, char* notate) {
//   char origChar = *posChar;
//   char* ptr = notate;
//   while (*ptr < *posChar && *ptr != '\0') { ptr++; }
//   if (*ptr == *posChar) { return; }  // place being made
//   unsigned char parity = ((ptr - notate) ^ (*posChar - '1')) % 2; // ok this simplification seems to work (moving out the common %2):
//   *posChar -= parity * 2 - 1;  // left or right movement
//   printf("pos: %c  PN: %s  updated pos: %c\n", origChar, notate, *posChar);
// }

// ascii for bells:
// "123456789:;<=>?@"
// note:
//    : is 10
//    ; is 11 (etc)
// .. @ is 16

#define ASSERT_EQ(expected, actual) do { \
  if ((expected) != (actual)) { \
    printf("  ******* ASSERT_EQ failed: %s != %s (%d != %d)\n", \
           #expected, #actual, (expected), (actual)); \
  } else { \
    printf("PASS: %s == %s (%d)\n", #expected, #actual, (expected)); \
  } \
} while (0)

#define ASSERT_STREQ(expected, actual) do { \
  if (strcmp((expected), (actual)) != 0) { \
    printf("  ******* ASSERT_STREQ failed: %s != %s (\"%s\" != \"%s\")\n", \
           #expected, #actual, (expected), (actual)); \
  } else { \
    printf("PASS: %s == %s (\"%s\")\n", #expected, #actual, (expected)); \
  } \
} while (0)

void updateBellPosChar(char *pos, char *notate) {
    // char origChar = *p;
    // char* origNotate = n;
  char *startNotate = notate;
  while (*notate < *pos && *notate) notate++; // step over notate until find a place >= to our bell position or '\0'
  if (*notate == *pos) return; // if making a place, do nothing
  *pos += ((notate - startNotate) ^ (*pos - '1')) & 1 ? -1 : 1; // step left or right based on parity
    // printf("pos: %c  PN: %s  updated pos: %c\n", origChar, origNotate, *p);
}

int parse_place_notation_sequence(const char* placeNotation, char placeNotates[MAX_TOKENS][MAX_TOKEN_LENGTH]) {
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

void assert_expanded_place_notation(const char* placeNotation, char* expected) {
  char result[255];
  char placeNotates[MAX_TOKENS][MAX_TOKEN_LENGTH];

  int token_count = parse_place_notation_sequence(placeNotation, placeNotates);

  concatenate_tokens(placeNotates, token_count, result, 255);

  ASSERT_STREQ(result, expected);
}

char h(char p, char *n) {
  updateBellPosChar(&p, n);
  return p;
}

void test_notate_row_processing() {
  // NOTE: we don't take the stage into account.
  // So stage 7, 'x' would cause '7' to become '8'.

  ASSERT_EQ(h('1', "x"), '2');
  ASSERT_EQ(h('2', "x"), '1');

  ASSERT_EQ(h('3', "x"), '4');
  ASSERT_EQ(h('4', "x"), '3');

  ASSERT_EQ(h('1', "12345678"), '1');
  ASSERT_EQ(h('2', "12345678"), '2');
  ASSERT_EQ(h('3', "12345678"), '3');
  ASSERT_EQ(h('4', "12345678"), '4');
  ASSERT_EQ(h('5', "12345678"), '5');
  ASSERT_EQ(h('6', "12345678"), '6');
  ASSERT_EQ(h('7', "12345678"), '7');
  ASSERT_EQ(h('8', "12345678"), '8');

  ASSERT_EQ(h('3', "12"), '4');
  ASSERT_EQ(h('4', "12"), '3');
  
  ASSERT_EQ(h('3', "14"), '2');
  ASSERT_EQ(h('2', "14"), '3');

  ASSERT_EQ(h('3', "1478"), '2');
  ASSERT_EQ(h('2', "1478"), '3');

  ASSERT_EQ(h('1', "1478"), '1');
  ASSERT_EQ(h('7', "1478"), '7');

  ASSERT_EQ(h('1', "78"), '2');
  ASSERT_EQ(h('2', "78"), '1');

  ASSERT_EQ(h('7', "1458"), '6');
  ASSERT_EQ(h('6', "1458"), '7');

  ASSERT_EQ(h('2', "1"), '3');
  ASSERT_EQ(h('3', "1"), '2');

  ASSERT_EQ(h('1', "7"), '2');
  ASSERT_EQ(h('2', "7"), '1');

  ASSERT_EQ(h('1', "3"), '2');
  ASSERT_EQ(h('2', "3"), '1');
}

void test_expand_place_notation() {
  // note the "|" placed between resulting notates, for convenience
  assert_expanded_place_notation("", "");
  assert_expanded_place_notation("x", "x|");
  assert_expanded_place_notation(".", "");
  assert_expanded_place_notation("18", "18|");
  assert_expanded_place_notation("18,12", "18|12|");
  assert_expanded_place_notation("18x38,12", "18|x|38|x|18|12|");
  assert_expanded_place_notation("x12x14,34", "x|12|x|14|x|12|x|34|");
}

void generate_single_bell_blueline() {

  // 
  //  - don't test this. Just assume we will never use x on odd stage and PN is reasonably well formed.
  //       XX add param for stage 7! XX
  // ASSERT_EQ(h('7', "x"), '7');

  // pb4
  // char *notates[] = {"x", "14", "x", "14", "x", "14", "x", "12"};

  // stedman
  char *notates[] = {"3", "1", "7", "3", "1", "3", "1", "3", "7", "1", "3", "1"};

  char b = '2';

  int pnCount = sizeof(notates) / sizeof(notates[0]);
  int pbs = 7;
  char* spaces = "          ";

  // printf("1\n");
  for (int i = 0; i < pnCount * pbs; i+= 1) {
    int pnIdx = i % pnCount;
    if (pnIdx == 0) {
      printf("------------\n");
    }
    updateBellPosChar(&b, notates[pnIdx]);
    // printf("%s\t%c\n", notates[i % pnCount], b);
    printf("  ");
    for (int space = 0; space < b - '1'; space++) {
      printf(" ");
    }
    // printf("*\n");
    printf("%c\n", b);
  }  
}

int main() {
  // test_notate_row_processing();
  // generate_single_bell_blueline();
  test_expand_place_notation();
  return 0;
}

