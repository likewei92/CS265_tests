# C-compiler settings
CC = gcc -std=c99 -g -ggdb3

# Flags and other libraries
CFLAGS += -Wall -Wextra -pedantic -pthread -O3

# Header files
DEPS = 

# common .o object files
OBJ = 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

select_pos: $(OBJ) select_pos.o
	$(CC) -o $@.out $^ $(CFLAGS)
	
select_bit: $(OBJ) select_bit.o
	$(CC) -o $@.out $^ $(CFLAGS)

select_pos_3way: $(OBJ) select_pos_3way.o
	$(CC) -o $@.out $^ $(CFLAGS)

all: select_pos select_bit select_pos_3way

clean:
	rm *.out *.o