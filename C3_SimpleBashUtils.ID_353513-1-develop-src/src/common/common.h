#ifndef S21_COMMON_H
#define S21_COMMON_H

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* Константы */
#define MAX_LINE_LENGTH 4096
#define INITIAL_FILE_CAPACITY 16
#define LINE_NUMBER_WIDTH 6

/* Коды возврата */
typedef enum {
  SUCCESS = 0,
  ERROR_GENERAL = 1,  // Общая ошибка (файл не найден, неправильная опция)
  ERROR_MEMORY_ALLOCATION = 2,  // Ошибка выделения памяти
  ERROR_FILE_READ = 3           // Ошибка чтения файла
} error_code_t;

/* Опции для cat */
typedef struct {
  int number_all;      /* -n: нумеровать все строки */
  int number_nonempty; /* -b: нумеровать непустые строки */
  int squeeze_blank; /* -s: сжимать множественные пустые строки */
  int show_ends; /* -e: показывать $ в конце строк */
  int show_tabs; /* -t: показывать табуляцию как ^I */
  int show_nonprinting; /* -v: показывать непечатаемые символы */
} cat_options_t;

/* Опции для grep */
typedef struct {
  int ignore_case;   /* -i: игнорировать регистр */
  int invert_match;  /* -v: инвертировать совпадения */
  int count_matches; /* -c: подсчитывать совпадения */
  int list_files; /* -l: выводить только имена файлов */
  int line_number; /* -n: выводить номера строк */
  int no_filename; /* -h: не выводить имена файлов */
  int suppress_errors; /* -s: подавлять сообщения об ошибках */
  int patterns_from_file; /* -f: получать шаблоны из файла */
  int only_matching; /* -o: выводить только совпадающие части */
} grep_options_t;

/* Список файлов */
typedef struct {
  char **files;
  int file_count;
  int capacity;
} file_list_t;

/* Функции для работы со списком файлов */
int init_file_list(file_list_t *file_list);
int add_file_to_list(file_list_t *file_list, const char *filename);
void free_file_list(file_list_t *file_list);

/* Функции для работы с опциями */
void init_cat_options(cat_options_t *opts);
void init_grep_options(grep_options_t *opts);

/* Утилиты для обработки ошибок */
void print_error(const char *program_name, const char *message);
void print_file_error(const char *program_name, const char *filename);
int handle_file_error(const char *program_name, const char *filename);

/* Утилиты для работы с файлами */
FILE *safe_fopen(const char *filename, const char *mode);
int safe_fclose(FILE *fp, const char *filename);

#endif /* S21_COMMON_H */