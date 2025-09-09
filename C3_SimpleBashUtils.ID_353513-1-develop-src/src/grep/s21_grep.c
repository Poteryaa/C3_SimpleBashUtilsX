#include "s21_grep.h"

/* === ФУНКЦИИ ДЛЯ ПАРСИНГА АРГУМЕНТОВ === */

/* Инициализация структур для парсинга */
int init_parse_structures(int argc, pattern_list_t *patterns,
                          char ***file_list) {
  patterns->pattern_count = 0;
  patterns->patterns = malloc(sizeof(char *) * argc);
  *file_list = malloc(sizeof(char *) * argc);

  if (!patterns->patterns || !*file_list) {
    fprintf(stderr, "grep: memory allocation failed\n");
    return ERROR_MEMORY_ALLOCATION;
  }

  return SUCCESS;
}

/* Парсинг одной опции */
int parse_single_option(char option, grep_options_t *opts) {
  switch (option) {
    case 'i':
      opts->ignore_case = 1;
      break;
    case 'v':
      opts->invert_match = 1;
      break;
    case 'c':
      opts->count_matches = 1;
      break;
    case 'l':
      opts->list_files = 1;
      break;
    case 'n':
      opts->line_number = 1;
      break;
    case 'h':
      opts->no_filename = 1;
      break;
    case 's':
      opts->suppress_errors = 1;
      break;
    case 'o':
      opts->only_matching = 1;
      break;
    default:
      fprintf(stderr, "grep: invalid option -- '%c'\n", option);
      return ERROR_GENERAL;
  }
  return SUCCESS;
}

/* Парсинг строки опций */
int parse_option_string(const char *option_str, grep_options_t *opts) {
  for (int j = 1; option_str[j] != '\0'; j++) {
    int result = parse_single_option(option_str[j], opts);
    if (result != SUCCESS) {
      return result;
    }
  }
  return SUCCESS;
}

/* Добавление шаблона в список */
int add_pattern_to_list(pattern_list_t *patterns, const char *pattern) {
  patterns->patterns[patterns->pattern_count] = strdup(pattern);
  if (!patterns->patterns[patterns->pattern_count]) {
    fprintf(stderr, "grep: memory allocation failed\n");
    return ERROR_MEMORY_ALLOCATION;
  }
  patterns->pattern_count++;
  return SUCCESS;
}

/* Обработка опции -e */
int handle_e_option(char *pattern_arg, pattern_list_t *patterns) {
  if (!pattern_arg) {
    fprintf(stderr, "grep: option requires an argument -- e\n");
    return ERROR_GENERAL;
  }

  return add_pattern_to_list(patterns, pattern_arg);
}

/* Обработка опции -f */
int handle_f_option(const char *filename, pattern_list_t *patterns,
                    grep_options_t *opts) {
  FILE *pat_file = fopen(filename, "r");
  if (!pat_file) {
    if (!opts->suppress_errors) {
      fprintf(stderr, "grep: %s: No such file or directory\n", filename);
    }
    return ERROR_GENERAL;
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read;

  while ((read = getline(&line, &len, pat_file)) != -1) {
    if (read > 0 && line[read - 1] == '\n') {
      line[read - 1] = '\0';
    }

    int result = add_pattern_to_list(patterns, line);
    if (result != SUCCESS) {
      free(line);
      fclose(pat_file);
      return result;
    }
  }

  free(line);
  fclose(pat_file);
  return SUCCESS;
}

/* Очистка при выходе с ошибкой */
void cleanup_and_exit(pattern_list_t *patterns, char **file_list,
                      int exit_code) {
  free_patterns(patterns);
  free(file_list);
  exit(exit_code);
}

/* Основная функция парсинга аргументов */
void parse_args(int argc, char *argv[], grep_options_t *opts, char ***files,
                int *file_count, pattern_list_t *patterns) {
  char **file_list;
  int pattern_found = 0;
  int file_idx = 0;

  if (init_parse_structures(argc, patterns, &file_list) != SUCCESS) {
    exit(1);
  }

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-e") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "grep: option requires an argument -- e\n");
        cleanup_and_exit(patterns, file_list, 2);
      }

      if (handle_e_option(argv[++i], patterns) != SUCCESS) {
        cleanup_and_exit(patterns, file_list, 1);
      }
      pattern_found = 1;

    } else if (strcmp(argv[i], "-f") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "grep: option requires an argument -- f\n");
        cleanup_and_exit(patterns, file_list, 2);
      }

      if (handle_f_option(argv[++i], patterns, opts) != SUCCESS) {
        cleanup_and_exit(patterns, file_list, 2);
      }
      pattern_found = 1;

    } else if (argv[i][0] == '-') {
      if (parse_option_string(argv[i], opts) != SUCCESS) {
        cleanup_and_exit(patterns, file_list, 2);
      }

    } else {
      if (!pattern_found && patterns->pattern_count == 0) {
        if (add_pattern_to_list(patterns, argv[i]) != SUCCESS) {
          cleanup_and_exit(patterns, file_list, 1);
        }
        pattern_found = 1;
      } else {
        file_list[file_idx++] = argv[i];
      }
    }
  }

  if (patterns->pattern_count == 0) {
    fprintf(stderr, "grep: no pattern\n");
    cleanup_and_exit(patterns, file_list, 2);
  }

  *files = file_list;
  *file_count = file_idx;
}

/* === ФУНКЦИИ ДЛЯ ОБРАБОТКИ ФАЙЛОВ === */

/* Безопасное открытие файла */
FILE *safe_fopen_grep(const char *filename, const char *mode) {
  return strcmp(filename, "-") == 0 ? stdin : fopen(filename, mode);
}

/* Безопасное закрытие файла */
void safe_fclose_grep(FILE *fp, const char *filename) {
  (void)filename;  // подавляем предупреждение о неиспользуемом параметре
  if (fp != stdin) {
    fclose(fp);
  }
}

/* Инициализация скомпилированных regex паттернов */
int init_regex_patterns(pattern_list_t patterns, grep_options_t opts,
                        compiled_patterns_t *compiled) {
  compiled->regexes = malloc(sizeof(regex_t) * patterns.pattern_count);
  compiled->empty_patterns = malloc(sizeof(int) * patterns.pattern_count);
  compiled->pattern_count = patterns.pattern_count;

  if (!compiled->regexes || !compiled->empty_patterns) {
    fprintf(stderr, "grep: memory allocation failed\n");
    return ERROR_MEMORY_ALLOCATION;
  }

  for (int i = 0; i < patterns.pattern_count; i++) {
    compiled->empty_patterns[i] = (strlen(patterns.patterns[i]) == 0);

    if (!compiled->empty_patterns[i]) {
      int flags = REG_EXTENDED;
      if (opts.ignore_case) flags |= REG_ICASE;

      if (regcomp(&compiled->regexes[i], patterns.patterns[i], flags) != 0) {
        fprintf(stderr, "grep: invalid pattern\n");
        for (int j = 0; j < i; j++) {
          if (!compiled->empty_patterns[j]) {
            regfree(&compiled->regexes[j]);
          }
        }
        free(compiled->regexes);
        free(compiled->empty_patterns);
        return ERROR_GENERAL;
      }
    }
  }
  return SUCCESS;
}

/* Очистка ресурсов regex */
void cleanup_regex_resources(compiled_patterns_t *compiled) {
  for (int i = 0; i < compiled->pattern_count; i++) {
    if (!compiled->empty_patterns[i]) {
      regfree(&compiled->regexes[i]);
    }
  }
  free(compiled->regexes);
  free(compiled->empty_patterns);
}

/* Проверка совпадения строки с паттернами */
int check_line_match(const char *line, compiled_patterns_t *compiled) {
  for (int i = 0; i < compiled->pattern_count; i++) {
    if (compiled->empty_patterns[i]) {
      return 1;
    }
    if (regexec(&compiled->regexes[i], line, 0, NULL, 0) == 0) {
      return 1;
    }
  }
  return 0;
}

/* Вывод префикса для совпадения */
void print_match_prefix(const char *filename, int line_num, int multiple_files,
                        grep_options_t opts) {
  if (multiple_files && !opts.no_filename) {
    printf("%s:", filename);
  }
  if (opts.line_number) {
    printf("%d:", line_num);
  }
}

/* Вывод результатов подсчета */
void print_count_results(const char *filename, int match_count,
                         int multiple_files, grep_options_t opts) {
  if (multiple_files && !opts.no_filename) {
    printf("%s:%d\n", filename, match_count);
  } else {
    printf("%d\n", match_count);
  }
}

/* Обработка обычного совпадения */
void handle_regular_match(const char *line, const char *filename, int line_num,
                          int multiple_files, grep_options_t opts) {
  if (!opts.count_matches && !opts.list_files) {
    print_match_prefix(filename, line_num, multiple_files, opts);
    printf("%s\n", line);
  }
}

/* Обработка опции -o (only matching) */
int handle_only_matching(const char *line, size_t line_length,
                         compiled_patterns_t *compiled, grep_options_t opts,
                         const char *filename, int line_num,
                         int multiple_files) {
  int matches = 0;

  if (opts.invert_match) {
    int line_matches = check_line_match(line, compiled);
    return !line_matches;
  }

  for (int i = 0; i < compiled->pattern_count; i++) {
    if (compiled->empty_patterns[i]) {
      matches = 1;
      break;
    }

    regmatch_t match;
    size_t offset = 0;
    int line_has_matches = 0;

    while (offset < line_length &&
           regexec(&compiled->regexes[i], line + offset, 1, &match, 0) == 0) {
      if (match.rm_so == match.rm_eo) {
        offset++;
        continue;
      }

      line_has_matches = 1;
      matches = 1;

      if (!opts.count_matches && !opts.list_files) {
        print_match_prefix(filename, line_num, multiple_files, opts);
        printf("%.*s\n", (int)(match.rm_eo - match.rm_so),
               line + offset + match.rm_so);
      }
      offset += match.rm_eo;
    }

    if (line_has_matches) {
      break;
    }
  }

  return matches;
}

/* Основная функция обработки файла */
int process_file(const char *filename, grep_options_t opts,
                 pattern_list_t patterns, int multiple_files,
                 int *error_occurred) {
  FILE *fp = safe_fopen_grep(filename, "r");
  if (!fp) {
    if (!opts.suppress_errors) {
      fprintf(stderr, "grep: %s: No such file or directory\n", filename);
    }
    *error_occurred = 1;
    return 0;
  }

  compiled_patterns_t compiled;
  if (init_regex_patterns(patterns, opts, &compiled) != SUCCESS) {
    safe_fclose_grep(fp, filename);
    exit(1);
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int line_num = 0;
  int match_count = 0;
  int has_match = 0;

  while ((read = getline(&line, &len, fp)) != -1) {
    line_num++;

    if (read > 0 && line[read - 1] == '\n') {
      line[read - 1] = '\0';
      read--;
    }

    int matches = 0;

    if (opts.only_matching) {
      matches = handle_only_matching(line, read, &compiled, opts, filename,
                                     line_num, multiple_files);
      if (matches) match_count++;
    } else {
      matches = check_line_match(line, &compiled);
      if (opts.invert_match) matches = !matches;

      if (matches) {
        match_count++;
        handle_regular_match(line, filename, line_num, multiple_files, opts);
      }
    }

    if (matches) has_match = 1;
  }

  if (opts.count_matches) {
    print_count_results(filename, match_count, multiple_files, opts);
  } else if (opts.list_files && has_match) {
    printf("%s\n", filename);
  }

  free(line);
  cleanup_regex_resources(&compiled);
  safe_fclose_grep(fp, filename);

  return match_count;
}

/* === УТИЛИТАРНЫЕ ФУНКЦИИ === */

/* Освобождение памяти паттернов */
void free_patterns(pattern_list_t *patterns) {
  for (int i = 0; i < patterns->pattern_count; i++) {
    free(patterns->patterns[i]);
  }
  free(patterns->patterns);
  patterns->patterns = NULL;
  patterns->pattern_count = 0;
}

/* Основная функция */
int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: grep [OPTION]... PATTERN [FILE]...\n");
    return 2;
  }

  grep_options_t opts = {0};
  char **files = NULL;
  int file_count = 0;
  pattern_list_t patterns = {NULL, 0};
  int error_occurred = 0;
  int total_matches_found = 0;

  parse_args(argc, argv, &opts, &files, &file_count, &patterns);

  // Подсчитываем количество существующих файлов
  int existing_files = 0;
  for (int i = 0; i < file_count; i++) {
    if (strcmp(files[i], "-") == 0) {
      existing_files++;
    } else {
      FILE *fp = fopen(files[i], "r");
      if (fp) {
        existing_files++;
        fclose(fp);
      } else if (!opts.suppress_errors) {
        fprintf(stderr, "grep: %s: No such file or directory\n", files[i]);
        error_occurred = 1;
      } else {
        error_occurred = 1;
      }
    }
  }

  int multiple_files = file_count > 1;

  if (file_count == 0) {
    total_matches_found =
        process_file("-", opts, patterns, multiple_files, &error_occurred);
  } else {
    for (int i = 0; i < file_count; i++) {
      int file_matches = process_file(files[i], opts, patterns, multiple_files,
                                      &error_occurred);
      total_matches_found += file_matches;
    }
  }

  free_patterns(&patterns);
  free(files);

  if (error_occurred) {
    return 2;
  }
  if (total_matches_found == 0) {
    return 1;
  }
  return 0;
}