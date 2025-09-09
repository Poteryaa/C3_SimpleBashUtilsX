#ifndef S21_CAT_H
#define S21_CAT_H

#include "../common/common.h"

/* Константы для обработки символов */
#define TAB_CHAR '\t'
#define NEWLINE_CHAR '\n'
#define CTRL_OFFSET 64
#define HIGH_BIT_MASK 128
#define CTRL_RANGE_END 159
#define PRINTABLE_START 32
#define EXTENDED_START 160

/* Функции для парсинга аргументов */
int parse_cat_arguments(int argc, char *argv[], cat_options_t *opts,
                        file_list_t *files);
int is_valid_cat_option(const char *option);
void set_cat_option(const char *option, cat_options_t *opts);
void resolve_cat_option_conflicts(cat_options_t *opts);

/* Функции для обработки файлов */
int process_cat_files(const file_list_t *files, const cat_options_t *opts);
int process_single_cat_file(const char *filename, const cat_options_t *opts);

/* Функции для обработки содержимого */
int process_file_line(const char *line, size_t length,
                      const cat_options_t *opts, int *line_counter,
                      int *empty_line_counter);
void print_line_number(int line_number);
int should_skip_empty_line(int empty_counter, int squeeze_blank);
int is_empty_line(const char *line, size_t length);

/* Функции для вывода символов */
void print_character_with_options(unsigned char c, const cat_options_t *opts);
void print_tab_character(void);
void print_end_character(void);
void print_nonprinting_character(unsigned char c);
void print_extended_character(unsigned char c);

/* Функция для поддержки GNU опций */
void preprocess_gnu_options(int argc, char *argv[]);

#endif /* S21_CAT_H */