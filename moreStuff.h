#ifndef MORESTUFF
#define MORESTUFF

#include "structs.h"

int csi_ending_char(char b);
void parse_csi(CS *cs);
void printCS(CS *cs);
void handle_csi(CS *cs);
void vtParse3(const char *p, int size, Term *term, CS *cs, void (*handle_csi)(CS *cs));

#endif
