#ifndef S21_GREP_H
#define S21_GREP_H

#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int ignore_case;    // -i: игнорировать регистр
  int invert_match;   // -v: инвертировать совпадения
  int count_matches;  // -c: подсчитывать совпадения
  int list_files;   // -l: выводить только имена файлов
  int line_number;  // -n: выводить номера строк
  int suppress_errors;  // -s: подавлять сообщения об ошибках
  int no_filename;  // -h: подавлять имена файлов
  int only_matching;  // -o: выводить только совпадающие части
} grep_options_t;

typedef struct {
  char **patterns;    // Массив шаблонов
  int pattern_count;  // Количество шаблонов
} pattern_list_t;

typedef struct {
  regex_t *regexes;
  int *empty_patterns;
  int pattern_count;
} compiled_patterns_t;

/* Основные функции */
void parse_args(int argc, char *argv[], grep_options_t *opts, char ***files,
                int *file_count, pattern_list_t *patterns);
int process_file(const char *filename, grep_options_t opts,
                 pattern_list_t patterns, int multiple_files,
                 int *error_occurred);
void free_patterns(pattern_list_t *patterns);

/* Функции для парсинга аргументов */
int init_parse_structures(int argc, pattern_list_t *patterns,
                          char ***file_list);
int parse_single_option(char option, grep_options_t *opts);
int parse_option_string(const char *option_str, grep_options_t *opts);
int handle_e_option(char *pattern_arg, pattern_list_t *patterns);
int handle_f_option(const char *filename, pattern_list_t *patterns,
                    grep_options_t *opts);
void cleanup_and_exit(pattern_list_t *patterns, char **file_list,
                      int exit_code);
int add_pattern_to_list(pattern_list_t *patterns, const char *pattern);

/* Функции для обработки файлов */
int init_regex_patterns(pattern_list_t patterns, grep_options_t opts,
                        compiled_patterns_t *compiled);
void cleanup_regex_resources(compiled_patterns_t *compiled);
int check_line_match(const char *line, compiled_patterns_t *compiled);
int handle_only_matching(const char *line, size_t line_length,
                         compiled_patterns_t *compiled, grep_options_t opts,
                         const char *filename, int line_num,
                         int multiple_files);
void print_match_prefix(const char *filename, int line_num, int multiple_files,
                        grep_options_t opts);
void handle_regular_match(const char *line, const char *filename, int line_num,
                          int multiple_files, grep_options_t opts);
void print_count_results(const char *filename, int match_count,
                         int multiple_files, grep_options_t opts);
FILE *safe_fopen_grep(const char *filename, const char *mode);
void safe_fclose_grep(FILE *fp, const char *filename);

/* Константы */
#define SUCCESS 0
#define ERROR_GENERAL 1
#define ERROR_MEMORY_ALLOCATION 2

#endif