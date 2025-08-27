#include "common.h"

void parse_files_and_options(int argc, char *argv[], void *opts,
                             const char *valid_options[], int valid_options_count,
                             file_list_t *file_list) {
  file_list->file_count = 0;

  // Подсчитываем количество файлов
  int potential_files = 0;
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      potential_files++;
    }
  }

  // Выделяем память для файлов
  if (potential_files > 0) {
    file_list->files = malloc(sizeof(char*) * potential_files);
    if (!file_list->files) {
      fprintf(stderr, "memory allocation failed\n");
      exit(1);
    }
  } else {
    file_list->files = NULL;
  }

  // Разбираем аргументы
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-') {
      int valid = 0;
      for (int j = 0; j < valid_options_count; j++) {
        if (strcmp(argv[i], valid_options[j]) == 0) {
          valid = 1;
          // Устанавливаем флаги для s21_cat
          if (strcmp(argv[i], "-n") == 0) {
            ((options_t*)opts)->number_all = 1;
            ((grep_options_t*)opts)->line_number = 1;
          } else if (strcmp(argv[i], "-b") == 0) {
            ((options_t*)opts)->number_nonempty = 1;
          } else if (strcmp(argv[i], "-s") == 0) {
            ((options_t*)opts)->squeeze_blank = 1;
          } else if (strcmp(argv[i], "-e") == 0) {
            ((options_t*)opts)->show_ends = 1;
            ((options_t*)opts)->show_nonprinting = 1;
          } else if (strcmp(argv[i], "-t") == 0) {
            ((options_t*)opts)->show_tabs = 1;
            ((options_t*)opts)->show_nonprinting = 1;
          }
          // Устанавливаем флаги для s21_grep
          else if (strcmp(argv[i], "-i") == 0) {
            ((grep_options_t*)opts)->ignore_case = 1;
          } else if (strcmp(argv[i], "-v") == 0) {
            ((grep_options_t*)opts)->invert_match = 1;
          } else if (strcmp(argv[i], "-c") == 0) {
            ((grep_options_t*)opts)->count_matches = 1;
          } else if (strcmp(argv[i], "-l") == 0) {
            ((grep_options_t*)opts)->list_files = 1;
          }
          break;
        }
      }
      if (!valid) {
        fprintf(stderr, "cat: invalid option -- '%c'\nTry 'cat --help' for more information.\n", argv[i][1]);
        free(file_list->files);
        exit(1);
      }
    } else {
      file_list->files[file_list->file_count++] = argv[i];
    }
  }
}

void free_file_list(file_list_t *file_list) {
  free(file_list->files);
  file_list->files = NULL;
  file_list->file_count = 0;
}