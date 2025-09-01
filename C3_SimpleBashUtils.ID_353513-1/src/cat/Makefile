CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c11 -D_POSIX_C_SOURCE=200809L

s21_cat: s21_cat.o
	$(CC) $(CFLAGS) -o s21_cat s21_cat.o

s21_cat.o: s21_cat.c s21_cat.h
	$(CC) $(CFLAGS) -c -o s21_cat.o s21_cat.c

clean:
	rm -f s21_cat s21_cat.o

.PHONY: clean