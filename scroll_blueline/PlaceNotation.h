#ifndef PLACE_NOTATION_H
#define PLACE_NOTATION_H

#define MAX_ROW_LENGTH 11  // Max 10 bells + null terminator
#define MAX_METHOD_TITLE_LENGTH 20
// enough for bristol if using ',' to reverse
#define MAX_METHOD_PLACE_NOTATION_LENGTH 40
// place notation array
#define MAX_TOKENS 35
// can handle max 4 char notate like 1256 (the last char is for
// null temrinator; could get rid of need for that eventually with helper func)
#define MAX_TOKEN_LENGTH 5

extern void updateBellPosChar(char *posChar, char* notate);
extern void apply_place_notation(char* row, const char* notation);
extern int parse_place_notation_sequence(const char* placeNotation, char placeNotates[][MAX_TOKEN_LENGTH]);

#endif
