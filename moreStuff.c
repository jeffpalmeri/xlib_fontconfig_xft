// Does not require any extern stuff to work

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"
#include "moreStuff.h"

// function pointer is not needed any more
// void vtParse3(const char *p, int size, void (*wc)(const char*)) {
// void vtParse3(const char *p, int size, Term *term, CS *cs) {
void vtParse3(const char *p, int size, Term *term, CS *cs, void (*handle_csi)(CS *cs)) {
  for (int i = 0; i < size; i++) {
    char b = p[i];
    if (term->esc == 0) {
      if (b == 0x1b) {
        term->esc |= ESC_START;
        continue;
      }

      // write normal char here?
      // write_char(p+i);
      // wc(p+i);
      printf("cursor_x is %u and cursor_y is %u\n", term->cursor_x, term->cursor_y);
      printf("And the current char is %d\n", *(p+i));
      if(*(p+i) == 10) {
        printf("THIS IS A NEWLINE!!!!!!!\n");
        // Don't actually print a new line
        // Move our x position down a row.
        term->cursor_x++;
        term->cursor_y = 0;
      } else if(*(p+i) == 13) {
        printf("THIS IS A CARRIAGE RETURN!!!!!!!\n");
        // Don't actually print a new line
        // Move our x position down a row.
        // term.cursor_x++;
        // term.cursor_y = 100;
      } else if(*(p+i) == 9) {
        printf("THIS IS A HORIZONTAL TAB!!!!!!!\n");
      } else {
        term->lines[term->cursor_x][term->cursor_y] = (Line){
          .row = term->cursor_x,
          .col = term->cursor_y,
          .dirty = 1,
          .c = *(p+i),
        };
        if(term->cursor_y-+ 1 >= term->cols) {
          term->cursor_y = 0;
          term->cursor_x++;
        } else {
          term->cursor_y++;
        }
      }
      continue;
    }
    if (term->esc & ESC_START) {
      if (b == 0x5b) {
        term->esc |= ESC_CSI;
        continue;
      }
    }
    if (term->esc && ESC_CSI) {
      cs->buf[cs->len++] = b;
      if (csi_ending_char(b)) { // 64-126
        term->esc = 0;
        parse_csi(cs);
        printf("PRINTING CS!!\n");
        printCS(cs);
        handle_csi(cs);
      }
    }
  }
  // cs.buf[cs.len] = '\0';
  printf("DONEZOOOOOOO\n");
}

int csi_ending_char(char b) { return b > 64 && b < 126; }

void handle_csi(CS *cs) {
  switch (cs->mode[0]) {
  case 'h':
    // turn on bracketed paste mode (will implement later)
    break;
  case 'l':
    // turn off bracketed paste mode (will implement later)
    break;
  case 'm':
    break;
  }
  memset(cs, 0, sizeof(*cs));
}

void printCS(CS *cs) {
  printf("CS {\n");
  printf("  buf: %s\n", cs->buf);
  printf("  priv: %d\n", cs->priv);
  printf("  len: %d\n", cs->len);
  printf("  narg: %d\n", cs->narg);
  printf("  mode: %s\n", cs->mode);
  printf("  args: {\n");
  for (int i = 0; i < cs->narg; i++) {
    printf("    [%d]: %d\n", i, cs->arg[i]);
  }
  printf("  }\n");
  printf("}\n");
}


void parse_csi(CS *cs) {
  char sep = ';';
  cs->buf[cs->len] = '\0';
  char *p = cs->buf, *np;
  if (p[0] == '?') {
    cs->priv = '?';
    p++;
  }
  long v;
  // buf = something like 2004h
  //                          ^ <-- where strtol should stop
  while (p < cs->buf + cs->len) {
    /*
     * - buf could look something like this: "?2004h" or "0;4;5m"
     * - p starts pointing to the beginning of the buf.
     * - np starts as pointing to nothing.
     * - strtol will parse the first number and then np will point
     *   to where it stopped, which should be either ";" or the
     *   final character
     * - If p == np after strtol that means p was not pointing to
     *   a valid number to begin with so nothing happened.
     * - Otherwise, we did parse an arg and can save that in
     *   cs.arg.
     * - Then move p to be at the same place as np.
     * - If there are multiple args, then p will point to ';'
     *   and then increment p and loop continues.
     * - Otherwise we've reached the end of the buf and break
     * - Finally, set cs.mode
     */
    np = NULL;

    v = strtol(p, &np, 10);
    if (np == p) {
      // strtol didn't move
      v = 0;
    }

    cs->arg[cs->narg++] = v;

    p = np;
    if (*p != sep)
      break;
    p++;
  }

  cs->mode[0] = *p;
  cs->mode[1] = '\0'; // Can there be a second? st has a check for it. I can add
                     // that later if needed
}
