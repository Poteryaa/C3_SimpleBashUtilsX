#include "s21_cat.h"

void parse_args(int argc, char *argv[], options_t *opts, char ***files,
                int *file_count) {
  *file_count = 0;
  *files = NULL;

  // Подсчитываем файлы
  int potential_files = 0;
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] != '-') {
      potential_files++;
    }
  }

  // Выделяем память для файлов
  if (potential_files > 0) {
    *files = malloc(sizeof(char *) * potential_files);
    if (!*files) {
      fprintf(stderr, "memory allocation failed\n");
      exit(1);
    }
  }

  // Парсим аргументы
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && strlen(argv[i]) > 1) {
      if (strcmp(argv[i], "-b") == 0) {
        opts->number_nonempty = 1;
      } else if (strcmp(argv[i], "-n") == 0) {
        opts->number_all = 1;
      } else if (strcmp(argv[i], "-s") == 0) {
        opts->squeeze_blank = 1;
      } else if (strcmp(argv[i], "-e") == 0) {
        opts->show_ends = 1;
        opts->show_nonprinting = 1;
      } else if (strcmp(argv[i], "-t") == 0) {
        opts->show_tabs = 1;
        opts->show_nonprinting = 1;
      } else {
        fprintf(stderr,
                "cat: invalid option -- '%c'\nTry 'cat --help' for more "
                "information.\n",
                argv[i][1]);
        if (*files) free(*files);
        exit(1);
      }
    } else {
      (*files)[(*file_count)++] = argv[i];
    }
  }

  // GNU cat: приоритет для -b над -n
  if (opts->number_all && opts->number_nonempty) {
    opts->number_all = 0;
  }
}

void print_char_with_options(unsigned char c, options_t opts) {
  if (opts.show_ends && c == '\n') {
    printf("$");
    putchar(c);
  } else if (opts.show_tabs && c == '\t') {
    printf("^I");
  } else if (opts.show_nonprinting && c < 32 && c != '\n' && c != '\t') {
    printf("^%c", c + 64);
  } else if (opts.show_nonprinting && c >= 128 && c <= 159) {
    printf("M-^%c", c - 128 + 64);
  } else if (opts.show_nonprinting && c >= 160) {
    printf("M-%c", c - 128);
  } else {
    putchar(c);
  }
}

void process_file(const char *filename, options_t opts, int *error_occurred) {
  FILE *fp = strcmp(filename, "-") == 0 ? stdin : fopen(filename, "r");
  if (!fp) {
    fprintf(stderr, "cat: %s: No such file or directory\n", filename);
    *error_occurred = 1;
    return;
  }

  char *line = NULL;
  size_t len = 0;
  ssize_t read;
  int line_num = 1;
  int consecutive_empty = 0;

  while ((read = getline(&line, &len, fp)) != -1) {
    // Определяем, пустая ли строка
    int is_empty = (read == 1 && line[0] == '\n');

    // Обработка флага -s (сжатие пустых строк)
    if (opts.squeeze_blank && is_empty) {
      consecutive_empty++;
      if (consecutive_empty > 1) {
        continue;  // Пропускаем лишние пустые строки
      }
    } else {
      consecutive_empty = is_empty ? 1 : 0;
    }

    // Нумерация строк
    if ((opts.number_nonempty && !is_empty) || opts.number_all) {
      printf("%6d\t", line_num++);
    } else if (opts.number_all) {
      line_num++;
    }

    // Обработка содержимого строки
    for (ssize_t i = 0; i < read; i++) {
      print_char_with_options((unsigned char)line[i], opts);
    }
  }

  if (line) {
    free(line);
  }
  if (fp != stdin) fclose(fp);
}

int main(int argc, char *argv[]) {
  options_t opts = {0};
  char **files = NULL;
  int file_count = 0;
  int error_occurred = 0;

  parse_args(argc, argv, &opts, &files, &file_count);

  if (file_count == 0) {
    process_file("-", opts, &error_occurred);
  } else {
    for (int i = 0; i < file_count; i++) {
      process_file(files[i], opts, &error_occurred);
    }
    free(files);
  }

  return error_occurred ? 1 : 0;
}