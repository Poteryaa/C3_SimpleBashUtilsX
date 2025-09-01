CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L

s21_grep: s21_grep.o
	$(CC) $(CFLAGS) -o s21_grep s21_grep.o

s21_grep.o: s21_grep.c s21_grep.h
	$(CC) $(CFLAGS) -c -o s21_grep.o s21_grep.c

clean:
	rm -f s21_grep s21_grep.o

.PHONY: clean