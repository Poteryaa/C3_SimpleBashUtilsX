#ifndef S21_CAT_H
#define S21_CAT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int number_all;       // -n: нумеровать все строки
  int number_nonempty;  // -b: нумеровать непустые строки
  int squeeze_blank;    // -s: сжимать множественные пустые строки
  int show_ends;        // -e: показывать $ в конце строк
  int show_tabs;        // -t: показывать табуляцию как ^I
  int show_nonprinting; // -e, -t: показывать непечатаемые символы
} options_t;

void parse_args(int argc, char *argv[], options_t *opts, char ***files,
               int *file_count);
void process_file(const char *filename, options_t opts, int *error_occurred);
void print_char_with_options(unsigned char c, options_t opts);

#endif