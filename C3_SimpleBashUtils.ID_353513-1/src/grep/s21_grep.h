#ifndef S21_GREP_H
#define S21_GREP_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>

typedef struct {
  int ignore_case;      // -i: игнорировать регистр
  int invert_match;     // -v: инвертировать совпадения
  int count_matches;    // -c: подсчитывать совпадения
  int list_files;       // -l: выводить только имена файлов
  int line_number;      // -n: выводить номера строк
  int suppress_errors;  // -s: подавлять сообщения об ошибках
} grep_options_t;

typedef struct {
  char **patterns; // Массив шаблонов
  int pattern_count; // Количество шаблонов
} pattern_list_t;

void parse_args(int argc, char *argv[], grep_options_t *opts, char ***files,
               int *file_count, pattern_list_t *patterns);
int process_file(const char *filename, grep_options_t opts, pattern_list_t patterns,
                int multiple_files, int *error_occurred);
void free_patterns(pattern_list_t *patterns);

#endif