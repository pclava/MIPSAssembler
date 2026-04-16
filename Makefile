CC=gcc
IDIR=include
CFLAGS=-Wall -Wextra -I$(IDIR) -g

SRC := $(wildcard src/*.c)
SRC := $(filter-out src/linker.c, $(SRC))
OBJ := $(SRC:.c=.o)

LINKER_SRC := src/linker.c src/mof.c src/symbol_table.c src/strings.c src/utils.c src/text.c src/mman.c
LINKER_OBJ := $(LINKER_SRC:.c=.o)

all: MIPSAssembler MIPSLinker

MIPSAssembler: $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o MIPSAssembler

MIPSLinker: $(LINKER_OBJ)
	$(CC) $(LINKER_OBJ) -o MIPSLinker

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f src/*.o