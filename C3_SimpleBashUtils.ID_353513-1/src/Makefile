CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L
CAT_DIR = cat
GREP_DIR = grep

all: s21_cat s21_grep

s21_cat: $(CAT_DIR)/s21_cat.o
	$(CC) $(CFLAGS) -o $(CAT_DIR)/s21_cat $(CAT_DIR)/s21_cat.o

s21_grep: $(GREP_DIR)/s21_grep.o
	$(CC) $(CFLAGS) -o $(GREP_DIR)/s21_grep $(GREP_DIR)/s21_grep.o

$(CAT_DIR)/s21_cat.o: $(CAT_DIR)/s21_cat.c $(CAT_DIR)/s21_cat.h
	$(CC) $(CFLAGS) -c -o $(CAT_DIR)/s21_cat.o $(CAT_DIR)/s21_cat.c

$(GREP_DIR)/s21_grep.o: $(GREP_DIR)/s21_grep.c $(GREP_DIR)/s21_grep.h
	$(CC) $(CFLAGS) -c -o $(GREP_DIR)/s21_grep.o $(GREP_DIR)/s21_grep.c

test: s21_cat s21_grep
	chmod +x test_s21_cat.sh test_s21_grep.sh
	./test_s21_cat.sh
	./test_s21_grep.sh

clean:
	rm -f $(CAT_DIR)/s21_cat $(CAT_DIR)/s21_cat.o $(GREP_DIR)/s21_grep $(GREP_DIR)/s21_grep.o

.PHONY: all test clean