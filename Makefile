# C-compiler settings
CC = gcc -std=c99 -g -ggdb3

# Flags and other libraries
CFLAGS += -Wall -Wextra -pedantic -pthread -O3

# Header files
DEPS = 

# .o object files
OBJ = select_test.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

select_test: $(OBJ)
	gcc -o $@.out $^ $(CFLAGS)

all: select_test
