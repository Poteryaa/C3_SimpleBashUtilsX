#include "common.h"

/* Инициализация списка файлов */
int init_file_list(file_list_t *file_list) {
  if (!file_list) {
    return ERROR_MEMORY_ALLOCATION;
  }

  file_list->files = malloc(sizeof(char *) * INITIAL_FILE_CAPACITY);
  if (!file_list->files) {
    return ERROR_MEMORY_ALLOCATION;
  }

  file_list->file_count = 0;
  file_list->capacity = INITIAL_FILE_CAPACITY;
  return SUCCESS;
}

/* Добавление файла в список */
int add_file_to_list(file_list_t *file_list, const char *filename) {
  if (!file_list || !filename) {
    return ERROR_MEMORY_ALLOCATION;
  }

  /* Увеличиваем размер массива при необходимости */
  if (file_list->file_count >= file_list->capacity) {
    file_list->capacity *= 2;
    char **new_files =
        realloc(file_list->files, sizeof(char *) * file_list->capacity);
    if (!new_files) {
      return ERROR_MEMORY_ALLOCATION;
    }
    file_list->files = new_files;
  }

  file_list->files[file_list->file_count] = (char *)filename;
  file_list->file_count++;
  return SUCCESS;
}

/* Освобождение памяти списка файлов */
void free_file_list(file_list_t *file_list) {
  if (file_list && file_list->files) {
    free(file_list->files);
    file_list->files = NULL;
    file_list->file_count = 0;
    file_list->capacity = 0;
  }
}

/* Инициализация опций cat */
void init_cat_options(cat_options_t *opts) {
  if (opts) {
    memset(opts, 0, sizeof(cat_options_t));
  }
}

/* Инициализация опций grep */
void init_grep_options(grep_options_t *opts) {
  if (opts) {
    memset(opts, 0, sizeof(grep_options_t));
  }
}

/* Вывод общей ошибки */
void print_error(const char *program_name, const char *message) {
  if (program_name && message) {
    fprintf(stderr, "%s: %s\n", program_name, message);
  }
}

/* Вывод ошибки файла */
void print_file_error(const char *program_name, const char *filename) {
  if (program_name && filename) {
    fprintf(stderr, "%s: %s: %s\n", program_name, filename, strerror(errno));
  }
}

/* Обработка ошибки файла */
int handle_file_error(const char *program_name, const char *filename) {
  print_file_error(program_name, filename);
  return ERROR_GENERAL;  // Возвращаем 1 вместо 2
}

/* Безопасное открытие файла */
FILE *safe_fopen(const char *filename, const char *mode) {
  if (!filename || !mode) {
    return NULL;
  }

  if (strcmp(filename, "-") == 0) {
    return (strcmp(mode, "r") == 0) ? stdin : stdout;
  }

  return fopen(filename, mode);
}

/* Безопасное закрытие файла */
int safe_fclose(FILE *fp, const char *filename) {
  if (!fp) {
    return ERROR_FILE_READ;
  }

  /* Не закрываем stdin/stdout */
  if (fp == stdin || fp == stdout) {
    return SUCCESS;
  }

  if (fclose(fp) != 0) {
    if (filename) {
      print_file_error("file operation", filename);
    }
    return ERROR_FILE_READ;
  }

  return SUCCESS;
}