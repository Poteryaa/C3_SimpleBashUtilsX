#ifndef S21_COMMON_H
#define S21_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int number_all;       // -n: нумеровать все строки
  int number_nonempty;  // -b: нумеровать непустые строки
  int squeeze_blank;  // -s: сжимать множественные пустые строки
  int show_ends;  // -e: показывать $ в конце строк
  int show_tabs;  // -t: показывать табуляцию как ^I
  int show_nonprinting;  // -e, -t: показывать непечатаемые символы
} options_t;

typedef struct {
  int ignore_case;    // -i: игнорировать регистр
  int invert_match;   // -v: инвертировать совпадения
  int count_matches;  // -c: подсчитывать совпадения
  int list_files;   // -l: выводить только имена файлов
  int line_number;  // -n: выводить номера строк
} grep_options_t;

typedef struct {
  char **files;    // Массив имен файлов
  int file_count;  // Количество файлов
} file_list_t;

void parse_files_and_options(int argc, char *argv[], void *opts,
                             const char *valid_options[],
                             int valid_options_count, file_list_t *file_list);

void free_file_list(file_list_t *file_list);

#endif