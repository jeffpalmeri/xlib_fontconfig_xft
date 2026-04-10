// Does not require any extern stuff to work

#include "moreStuff.h"
#include "structs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int calc_top(Term *term) {
  if (term->offset < term->rows) {
    return 0;
  }
  return (term->cursor_x + 1) % term->rows;
}

// function pointer is not needed any more
// void vtParse3(const char *p, int size, void (*wc)(const char*)) {
// void vtParse3(const char *p, int size, Term *term, CS *cs) {
void vtParse3(const char *p, int size, Term *term, CS *cs,
              void (*handle_csi)(CS *cs)) {
  for (int i = 0; i < size; i++) {
    char b = p[i];
    if (term->esc == 0) {
      if (b == 0x1b) {
        term->esc |= ESC_START;
        continue;
      }

      // de_printf("cursor_x is %u and cursor_y is %u\n", term->cursor_x,
      // term->cursor_y); de_printf("And the current char is %d\n", *(p+i));
      int cur_top = calc_top(term);
      int x = term->cursor_x;
      term->lines[x]->dirty = 1;
      if (*(p + i) == 10) {
        // de_printf("THIS IS A NEWLINE!!!!!!!\n");
        // Don't actually print a new line
        // Move our x position down a row.
        term->cursor_x = (x + 1) % term->rows;
        for (int i = 0; i < term->cols; i++) {
          // Clear the terminal state row
          term->lines[(x + 1) % term->rows]->lineData[i].c = '\0';
          // Set the NEXT line as dirty so that it gets visually cleared
          // term->lines[(x_offset)%term->rows]->dirty = 1;
          // term->lines[(x_offset+1)%term->rows][i].dirty = 1;
        }
        term->lines[(x + 1) % term->rows]->dirty = 1;
        term->offset++;
        term->cursor_y = 0;
      } else if (*(p + i) == 13) {
        // de_printf("THIS IS A CARRIAGE RETURN!!!!!!!\n");
      } else if (*(p + i) == 9) {
        // de_printf("THIS IS A HORIZONTAL TAB!!!!!!!\n");
      } else {
        term->lines[x]->lineData[term->cursor_y] = (JGlyph){
            .row = x,
            .col = term->cursor_y,
            .c = *(p + i),
        };
        if (term->cursor_y + 1 >= term->cols) {
          term->cursor_y = 0;
          term->cursor_x = (x + 1) % term->rows;
          term->lines[(x + 1) % term->rows]->dirty = 1;
          term->offset++;
        } else {
          term->cursor_y++;
        }
      }
      int new_top = calc_top(term);
      if (cur_top != new_top) {
        for (int i = 0; i < term->rows; i++) {
          term->lines[i]->dirty = 1;
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
        de_printf("PRINTING CS!!\n");
        printCS(cs);
        handle_csi(cs);
      }
    }
  }
  // cs.buf[cs.len] = '\0';
  // de_printf("DONEZOOOOOOO\n");
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
  de_printf("CS {\n");
  de_printf("  buf: %s\n", cs->buf);
  de_printf("  priv: %d\n", cs->priv);
  de_printf("  len: %d\n", cs->len);
  de_printf("  narg: %d\n", cs->narg);
  de_printf("  mode: %s\n", cs->mode);
  de_printf("  args: {\n");
  for (int i = 0; i < cs->narg; i++) {
    de_printf("    [%d]: %d\n", i, cs->arg[i]);
  }
  de_printf("  }\n");
  de_printf("}\n");
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
