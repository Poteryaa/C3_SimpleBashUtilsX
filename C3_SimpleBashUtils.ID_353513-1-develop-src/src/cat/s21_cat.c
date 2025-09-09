#include "s21_cat.h"

/* Основная функция */
int main(int argc, char *argv[]) {
  cat_options_t options;
  file_list_t files;
  int result = SUCCESS;

  init_cat_options(&options);

  preprocess_gnu_options(argc, argv);

  if (init_file_list(&files) != SUCCESS) {
    print_error("s21_cat", "memory allocation failed");
    return ERROR_MEMORY_ALLOCATION;
  }

  result = parse_cat_arguments(argc, argv, &options, &files);
  if (result != SUCCESS) {
    free_file_list(&files);
    return result;
  }

  result = process_cat_files(&files, &options); 

  free_file_list(&files);
  return result;
}

/* Парсинг аргументов командной строки */
int parse_cat_arguments(int argc, char *argv[], cat_options_t *opts,
                        file_list_t *files) {
  if (!opts || !files) {
    return ERROR_MEMORY_ALLOCATION;
  }

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && strlen(argv[i]) > 1) {
      if (!is_valid_cat_option(argv[i])) {
        fprintf(stderr, "cat: invalid option -- '%c'\n", argv[i][1]);
        fprintf(stderr, "Try 'cat --help' for more information.\n");
        return ERROR_GENERAL;
      }
      set_cat_option(argv[i], opts);
    } else {
      if (add_file_to_list(files, argv[i]) != SUCCESS) {
        return ERROR_MEMORY_ALLOCATION;
      }
    }
  }

  resolve_cat_option_conflicts(opts);
  return SUCCESS;
}

/* Проверка валидности опции */
int is_valid_cat_option(const char *option) {
  const char *valid_options[] = {"-b", "-n", "-s", "-e", "-t"};
  const int options_count = sizeof(valid_options) / sizeof(valid_options[0]);

  for (int i = 0; i < options_count; i++) {
    if (strcmp(option, valid_options[i]) == 0) {
      return 1;
    }
  }
  return 0;
}

/* Установка опции */
void set_cat_option(const char *option, cat_options_t *opts) {
  if (!option || !opts) {
    return;
  }

  if (strcmp(option, "-b") == 0) {
    opts->number_nonempty = 1;
  } else if (strcmp(option, "-n") == 0) {
    opts->number_all = 1;
  } else if (strcmp(option, "-s") == 0) {
    opts->squeeze_blank = 1;
  } else if (strcmp(option, "-e") == 0) {
    opts->show_ends = 1;
    opts->show_nonprinting = 1;
  } else if (strcmp(option, "-t") == 0) {
    opts->show_tabs = 1;
    opts->show_nonprinting = 1;
  }
}

/* Разрешение конфликтов опций */
void resolve_cat_option_conflicts(cat_options_t *opts) {
  if (!opts) {
    return;
  }

  /* GNU cat: приоритет -b над -n */
  if (opts->number_all && opts->number_nonempty) {
    opts->number_all = 0;
  }
}

/* Обработка файлов */
int process_cat_files(const file_list_t *files, const cat_options_t *opts) {
  int result = SUCCESS;

  if (!files || !opts) {
    return ERROR_MEMORY_ALLOCATION;
  }

  if (files->file_count == 0) {
    result = process_single_cat_file("-", opts);
  } else {
    for (int i = 0; i < files->file_count; i++) {
      int file_result = process_single_cat_file(files->files[i], opts);
      if (file_result != SUCCESS && result == SUCCESS) {
        result = file_result;
      }
    }
  }

  return result;
}

/* Обработка одного файла */
int process_single_cat_file(const char *filename, const cat_options_t *opts) {
  FILE *file;
  char *line = NULL;
  size_t line_capacity = 0;
  ssize_t line_length;
  int line_counter = 1;
  int empty_line_counter = 0;
  int result = SUCCESS;

  if (!filename || !opts) {
    return ERROR_MEMORY_ALLOCATION;
  }

  file = safe_fopen(filename, "r");
  if (!file) {
    return handle_file_error("cat", filename);
  }

  while ((line_length = getline(&line, &line_capacity, file)) != -1) {
    int process_result = process_file_line(line, line_length, opts,
                                           &line_counter, &empty_line_counter);
    if (process_result != SUCCESS && result == SUCCESS) {
      result = process_result;
    }
  }

  if (line) {
    free(line);
  }

  int close_result = safe_fclose(file, filename);
  if (close_result != SUCCESS && result == SUCCESS) {
    result = close_result;
  }

  return result;
}

/* Обработка строки файла */
int process_file_line(const char *line, size_t length,
                      const cat_options_t *opts, int *line_counter,
                      int *empty_line_counter) {
  if (!line || !opts || !line_counter || !empty_line_counter) {
    return ERROR_MEMORY_ALLOCATION;
  }

  int is_empty = is_empty_line(line, length);

  /* Обработка флага -s */
  if (opts->squeeze_blank && is_empty) {
    (*empty_line_counter)++;
    if (should_skip_empty_line(*empty_line_counter, opts->squeeze_blank)) {
      return SUCCESS;
    }
  } else {
    *empty_line_counter = is_empty ? 1 : 0;
  }

  /* Нумерация строк */
  if ((opts->number_nonempty && !is_empty) || opts->number_all) {
    print_line_number((*line_counter)++);
  } else if (opts->number_all) {
    (*line_counter)++;
  }

  /* Вывод содержимого строки */
  for (size_t i = 0; i < length; i++) {
    print_character_with_options((unsigned char)line[i], opts);
  }

  return SUCCESS;
}

/* Вывод номера строки */
void print_line_number(int line_number) {
  printf("%*d\t", LINE_NUMBER_WIDTH, line_number);
}

/* Проверка, нужно ли пропустить пустую строку */
int should_skip_empty_line(int empty_counter, int squeeze_blank) {
  return squeeze_blank && empty_counter > 1;
}

/* Проверка, является ли строка пустой */
int is_empty_line(const char *line, size_t length) {
  return (length == 1 && line[0] == NEWLINE_CHAR);
}

/* Вывод символа с учетом опций */
void print_character_with_options(unsigned char c, const cat_options_t *opts) {
  if (!opts) {
    putchar(c);
    return;
  }

  if (opts->show_ends && c == NEWLINE_CHAR) {
    print_end_character();
    putchar(c);
  } else if (opts->show_tabs && c == TAB_CHAR) {
    print_tab_character();
  } else if (opts->show_nonprinting && c < PRINTABLE_START &&
             c != NEWLINE_CHAR && c != TAB_CHAR) {
    print_nonprinting_character(c);
  } else if (opts->show_nonprinting && c >= HIGH_BIT_MASK &&
             c <= CTRL_RANGE_END) {
    print_extended_character(c);
  } else if (opts->show_nonprinting && c >= EXTENDED_START) {
    printf("M-%c", c - HIGH_BIT_MASK);
  } else {
    putchar(c);
  }
}

/* GNU long и short опции */
void preprocess_gnu_options(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--number-nonblank") == 0) {
            argv[i] = "-b";
        } else if (strcmp(argv[i], "--number") == 0) {
            argv[i] = "-n";  
        } else if (strcmp(argv[i], "--squeeze-blank") == 0) {
            argv[i] = "-s";
        }
        // ИСПРАВИТЬ ЭТИ СТРОКИ:
        else if (strcmp(argv[i], "-E") == 0) {
            argv[i] = "-e"; // Используем несуществующую опцию, которая будет проигнорирована
        } else if (strcmp(argv[i], "-T") == 0) {  
            argv[i] = "-t"; // Используем несуществующую опцию, которая будет проигнорирована
        }
    }
}

/* Вывод символа табуляции */
void print_tab_character(void) { printf("^I"); }

/* Вывод символа конца строки */
void print_end_character(void) { printf("$"); }

/* Вывод непечатаемого символа */
void print_nonprinting_character(unsigned char c) {
  printf("^%c", c + CTRL_OFFSET);
}

/* Вывод расширенного символа */
void print_extended_character(unsigned char c) {
  printf("M-^%c", c - HIGH_BIT_MASK + CTRL_OFFSET);
}
