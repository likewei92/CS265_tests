# C-compiler settings
CC = gcc -std=c99 -g -ggdb3

# Flags and other libraries
CFLAGS += -Wall -Wextra -pedantic -pthread -O3

# Header files
DEPS = 

# .o object files
OBJ = select_pos.o 

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

select_pos: $(OBJ)
	$(CC) -o $@.out $^ $(CFLAGS)
	
select_bit: $(OBJ)
	$(CC) -o $@.out $^ $(CFLAGS)

all: select_pos select_bit

clean:
	rm *.out *.o