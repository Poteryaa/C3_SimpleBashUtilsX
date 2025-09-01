#include "s21_grep.h"

void parse_args(int argc, char *argv[], grep_options_t *opts, char ***files,
                int *file_count, pattern_list_t *patterns) {
  patterns->pattern_count = 0;
  patterns->patterns = NULL;
  *files = NULL;
  *file_count = 0;

  int max_patterns = argc;
  int max_files = argc;

  patterns->patterns = malloc(sizeof(char *) * max_patterns);
  char **file_list = malloc(sizeof(char *) * max_files);

  if (!patterns->patterns || !file_list) {
    fprintf(stderr, "grep: memory allocation failed\n");
    exit(1);
  }

  int i = 1;
  int pattern_found = 0;
  int file_idx = 0;

  while (i < argc) {
    if (strcmp(argv[i], "-e") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "grep: option requires an argument -- e\n");
        free(patterns->patterns);
        free(file_list);
        exit(2);
      }
      patterns->patterns[patterns->pattern_count] = strdup(argv[++i]);
      if (!patterns->patterns[patterns->pattern_count]) {
        fprintf(stderr, "grep: memory allocation failed\n");
        free_patterns(patterns);
        free(file_list);
        exit(1);
      }
      patterns->pattern_count++;
      pattern_found = 1;
    } else if (strcmp(argv[i], "-f") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "grep: option requires an argument -- f\n");
        free_patterns(patterns);
        free(file_list);
        exit(2);
      }
      FILE *pat_file = fopen(argv[++i], "r");
      if (!pat_file) {
        if (!opts->suppress_errors) {
          fprintf(stderr, "grep: %s: No such file or directory\n", argv[i]);
        }
        free_patterns(patterns);
        free(file_list);
        exit(2);
      }
      char *line = NULL;
      size_t len = 0;
      ssize_t read;
      while ((read = getline(&line, &len, pat_file)) != -1) {
        if (read > 0 && line[read - 1] == '\n') {
          line[read - 1] = '\0';
          read--;
        }
        // ИСПРАВЛЕНИЕ: НЕ игнорируем пустые строки - они должны совпадать со
        // всеми строками
        patterns->patterns[patterns->pattern_count] = strdup(line);
        if (!patterns->patterns[patterns->pattern_count]) {
          fprintf(stderr, "grep: memory allocation failed\n");
          free(line);
          fclose(pat_file);
          free_patterns(patterns);
          free(file_list);
          exit(1);
        }
        patterns->pattern_count++;
        pattern_found = 1;
      }
      free(line);
      fclose(pat_file);
    } else if (argv[i][0] == '-') {
      for (int j = 1; argv[i][j] != '\0'; j++) {
        switch (argv[i][j]) {
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
            fprintf(stderr, "grep: invalid option -- '%c'\n", argv[i][j]);
            free_patterns(patterns);
            free(file_list);
            exit(2);
        }
      }
    } else {
      if (!pattern_found && patterns->pattern_count == 0) {
        patterns->patterns[patterns->pattern_count] = strdup(argv[i]);
        if (!patterns->patterns[patterns->pattern_count]) {
          fprintf(stderr, "grep: memory allocation failed\n");
          free_patterns(patterns);
          free(file_list);
          exit(1);
        }
        patterns->pattern_count++;
        pattern_found = 1;
      } else {
        file_list[file_idx++] = argv[i];
      }
    }
    i++;
  }

  if (patterns->pattern_count == 0) {
    fprintf(stderr, "grep: no pattern\n");
    free_patterns(patterns);
    free(file_list);
    exit(2);
  }

  *files = file_list;
  *file_count = file_idx;
}

int process_file(const char *filename, grep_options_t opts,
                 pattern_list_t patterns, int multiple_files,
                 int *error_occurred) {
  FILE *fp = strcmp(filename, "-") == 0 ? stdin : fopen(filename, "r");
  if (!fp) {
    if (!opts.suppress_errors) {
      fprintf(stderr, "grep: %s: No such file or directory\n", filename);
    }
    *error_occurred = 1;
    return 0;
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int line_num = 0;
  int match_count = 0;
  int has_match = 0;

  regex_t *regexes = malloc(sizeof(regex_t) * patterns.pattern_count);
  int *empty_patterns = malloc(sizeof(int) * patterns.pattern_count);
  if (!regexes || !empty_patterns) {
    fprintf(stderr, "grep: memory allocation failed\n");
    if (fp != stdin) fclose(fp);
    exit(1);
  }

  // ИСПРАВЛЕНИЕ: обработка пустых и непустых паттернов отдельно
  for (int i = 0; i < patterns.pattern_count; i++) {
    empty_patterns[i] = (strlen(patterns.patterns[i]) == 0);

    if (!empty_patterns[i]) {
      int flags = REG_EXTENDED;
      if (opts.ignore_case) flags |= REG_ICASE;
      if (regcomp(&regexes[i], patterns.patterns[i], flags) != 0) {
        fprintf(stderr, "grep: invalid pattern\n");
        for (int j = 0; j < i; j++) {
          if (!empty_patterns[j]) regfree(&regexes[j]);
        }
        free(regexes);
        free(empty_patterns);
        if (fp != stdin) fclose(fp);
        exit(2);
      }
    }
  }

  while ((read = getline(&line, &len, fp)) != -1) {
    line_num++;

    if (read > 0 && line[read - 1] == '\n') {
      line[read - 1] = '\0';
      read--;
    }

    int matches = 0;

    if (opts.only_matching && opts.invert_match) {
      for (int i = 0; i < patterns.pattern_count; i++) {
        if (empty_patterns[i]) {
          matches = 1;  // Пустой паттерн совпадает со всеми строками
          break;
        }
        if (regexec(&regexes[i], line, 0, NULL, 0) == 0) {
          matches = 1;
          break;
        }
      }
      matches = !matches;  // Инвертируем для -v

      if (matches) {
        match_count++;
        // НЕ ВЫВОДИМ НИЧЕГО при -o -v, только считаем совпадения
      }
    } else if (opts.only_matching && !opts.invert_match) {
      // Обычная логика для -o
      for (int i = 0; i < patterns.pattern_count; i++) {
        if (empty_patterns[i]) {
          // Пустой паттерн с -o не выводит ничего, но считается совпадением
          matches = 1;
          match_count++;
          break;
        }

        regmatch_t match;
        size_t offset = 0;
        int line_has_matches = 0;

        while (offset < (size_t)read &&
               regexec(&regexes[i], line + offset, 1, &match, 0) == 0) {
          if (match.rm_so == match.rm_eo) {
            offset++;
            continue;
          }

          line_has_matches = 1;
          matches = 1;

          if (!opts.count_matches && !opts.list_files) {
            if (multiple_files && !opts.no_filename) {
              printf("%s:", filename);
            }
            if (opts.line_number) {
              printf("%d:", line_num);
            }
            printf("%.*s\n", (int)(match.rm_eo - match.rm_so),
                   line + offset + match.rm_so);
          }
          offset += match.rm_eo;
        }

        if (line_has_matches) {
          match_count++;
          break;
        }
      }
    } else {
      // Обычная логика для всех остальных случаев
      for (int i = 0; i < patterns.pattern_count; i++) {
        if (empty_patterns[i]) {
          matches = 1;  // Пустой паттерн совпадает со всеми строками
          break;
        }
        if (regexec(&regexes[i], line, 0, NULL, 0) == 0) {
          matches = 1;
          break;
        }
      }
      if (opts.invert_match) matches = !matches;

      if (matches) {
        match_count++;
        if (!opts.count_matches && !opts.list_files) {
          if (multiple_files && !opts.no_filename) {
            printf("%s:", filename);
          }
          if (opts.line_number) {
            printf("%d:", line_num);
          }
          printf("%s\n", line);
        }
      }
    }

    if (matches) has_match = 1;
  }

  if (opts.count_matches) {
    // ИСПРАВЛЕНИЕ: при множественных файлах всегда показывать имя файла для -c
    // кроме случая когда явно указан -h
    if (multiple_files && !opts.no_filename) {
      printf("%s:%d\n", filename, match_count);
    } else {
      printf("%d\n", match_count);
    }
  } else if (opts.list_files && has_match) {
    printf("%s\n", filename);
  }

  free(line);
  for (int i = 0; i < patterns.pattern_count; i++) {
    if (!empty_patterns[i]) {
      regfree(&regexes[i]);
    }
  }
  free(regexes);
  free(empty_patterns);
  if (fp != stdin) fclose(fp);

  return match_count;
}

void free_patterns(pattern_list_t *patterns) {
  for (int i = 0; i < patterns->pattern_count; i++) {
    free(patterns->patterns[i]);
  }
  free(patterns->patterns);
  patterns->patterns = NULL;
  patterns->pattern_count = 0;
}

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
        // Если -s установлен, просто помечаем ошибку без вывода
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