CC = gcc
CFLAGS = -g -Wall -Wextra -Werror -std=c11 -pedantic -D_GNU_SOURCE
TARGET = s21_cat
SRCDIR = .
COMMONDIR = ../common
SOURCES = $(SRCDIR)/s21_cat.c
COMMON_SOURCES = $(COMMONDIR)/common.c
OBJECTS = $(SOURCES:.c=.o) $(COMMON_SOURCES:.c=.o)
INCLUDES = -I$(SRCDIR) -I$(COMMONDIR)

.PHONY: all clean fclean re

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJECTS)

fclean: clean
	rm -f $(TARGET)

re: fclean all

# Цели для тестирования
test: $(TARGET)
	./test_s21_cat.sh

# Цель для проверки стиля кода
style:
	clang-format -style=Google -n $(SOURCES) $(wildcard *.h)
	clang-format -style=Google -n $(COMMON_SOURCES) $(COMMONDIR)/*.h

# Цель для исправления стиля кода  
format:
	clang-format -style=Google -i $(SOURCES) $(wildcard *.h)
	clang-format -style=Google -i $(COMMON_SOURCES) $(COMMONDIR)/*.h