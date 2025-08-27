#include "s21_grep.h"

void parse_args(int argc, char *argv[], grep_options_t *opts, char ***files,
                int *file_count, pattern_list_t *patterns) {
  patterns->pattern_count = 0;
  patterns->patterns = NULL;
  *files = NULL;
  *file_count = 0;

  // Подсчитываем максимальное количество паттернов и файлов
  int max_patterns = argc; // максимально возможное количество
  int max_files = argc;
  
  // Выделяем память
  patterns->patterns = malloc(sizeof(char*) * max_patterns);
  char **file_list = malloc(sizeof(char*) * max_files);
  
  if (!patterns->patterns || !file_list) {
    fprintf(stderr, "grep: memory allocation failed\n");
    exit(1);
  }
  
  int i = 1;
  int pattern_found = 0;
  int file_idx = 0;
  
  while (i < argc) {
    if (strcmp(argv[i], "-e") == 0) {
      // Следующий аргумент - паттерн
      if (i + 1 >= argc) {
        fprintf(stderr, "grep: option requires an argument -- e\n");
        free(patterns->patterns);
        free(file_list);
        exit(2);
      }
      patterns->patterns[patterns->pattern_count++] = argv[++i];
      pattern_found = 1;
    } else if (argv[i][0] == '-') {
      // Обработка флагов
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
          default:
            fprintf(stderr, "grep: invalid option -- '%c'\n", argv[i][j]);
            free(patterns->patterns);
            free(file_list);
            exit(2);
        }
      }
    } else {
      // Не флаг и не -e
      if (!pattern_found && patterns->pattern_count == 0) {
        // Первый паттерн без -e
        patterns->patterns[patterns->pattern_count++] = argv[i];
        pattern_found = 1;
      } else {
        // Файл
        file_list[file_idx++] = argv[i];
      }
    }
    i++;
  }

  // Если паттерн не найден, это ошибка
  if (patterns->pattern_count == 0) {
    fprintf(stderr, "grep: no pattern\n");
    free(patterns->patterns);
    free(file_list);
    exit(2);
  }

  *files = file_list;
  *file_count = file_idx;
}

int process_file(const char *filename, grep_options_t opts, pattern_list_t patterns,
                int multiple_files, int *error_occurred) {
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
  if (!regexes) {
    fprintf(stderr, "grep: memory allocation failed\n");
    if (fp != stdin) fclose(fp);
    exit(1);
  }

  // Компилируем регулярные выражения
  for (int i = 0; i < patterns.pattern_count; i++) {
    int flags = REG_EXTENDED;
    if (opts.ignore_case) flags |= REG_ICASE;
    if (regcomp(&regexes[i], patterns.patterns[i], flags) != 0) {
      fprintf(stderr, "grep: invalid pattern\n");
      for (int j = 0; j < i; j++) regfree(&regexes[j]);
      free(regexes);
      if (fp != stdin) fclose(fp);
      exit(2);
    }
  }

  while ((read = getline(&line, &len, fp)) != -1) {
    line_num++;
    
    // Удаляем символ новой строки для обработки
    if (read > 0 && line[read - 1] == '\n') {
      line[read - 1] = '\0';
      read--;
    }
    
    int matches = 0;
    for (int i = 0; i < patterns.pattern_count; i++) {
      if (regexec(&regexes[i], line, 0, NULL, 0) == 0) {
        matches = 1;
        break;
      }
    }

    if (opts.invert_match) matches = !matches;
    
    if (matches) {
      has_match = 1;
      match_count++;
      
      if (!opts.count_matches && !opts.list_files) {
        if (multiple_files) {
          printf("%s:", filename);
        }
        if (opts.line_number) {
          printf("%d:", line_num);
        }
        printf("%s\n", line);
      }
    }
  }

  if (opts.count_matches) {
    if (multiple_files) {
      printf("%s:%d\n", filename, match_count);
    } else {
      printf("%d\n", match_count);
    }
  } else if (opts.list_files && has_match) {
    printf("%s\n", filename);
  }

  if (line) free(line);
    for (int i = 0; i < patterns.pattern_count; i++) {
        regfree(&regexes[i]);
    }
    free(regexes);
    if (fp != stdin) fclose(fp);
    
    return match_count;
}

void free_patterns(pattern_list_t *patterns) {
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

  if (file_count == 0) {
  total_matches_found = process_file("-", opts, patterns, 0, &error_occurred);
  } else {
    int multiple_files = file_count > 1;
    for (int i = 0; i < file_count; i++) {
      int file_matches = process_file(files[i], opts, patterns, multiple_files, &error_occurred);
      total_matches_found += file_matches;
    }
  }

  free_patterns(&patterns);
  free(files);
  if (error_occurred) {
    return 2;  // Ошибка (файл не найден и т.д.)
  }
  if (total_matches_found == 0) {
    return 1;  // Совпадений не найдено
  }

  return 0;
}