CC=gcc
IDIR=include
CFLAGS=-Wall -Wextra -I$(IDIR) -g

SRC := $(wildcard src/*.c)
OBJ := $(SRC:.c=.o)

all: MIPSAssembler

MIPSAssembler: $(OBJ)
	$(CC) $(OBJ) -o MIPSAssembler

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f src/*.o