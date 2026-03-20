#ifndef MIPS_ASSEMBLER_ASSEMBLER_H
#define MIPS_ASSEMBLER_ASSEMBLER_H

#include "symbol_table.h"
#include "instruction_parser.h"
#include "pseudoinstructions.h"
#include "instructions.h"
#include "reloc_table.h"
#include "data_parser.h"

/* === TYPES === */

typedef struct Assembler Assembler;

struct Assembler {
    Text *preprocessed;
    DataList *data_list;
    InstructionList *instruction_list;
    SymbolTable *symbol_table;
    InstructionTable *instruction_table;
    RelocationTable *relocation_table;
};

/* === ASSEMBLER STRUCTURE METHODS === */

int assembler_init(Assembler *assembler, Text *preprocessed);

void assembler_destroy(Assembler *assembler);

void assembler_debug(const Assembler *assembler);

/* === ASSEMBLY METHODS === */

int assembler_first_pass(Assembler *assembler);

int assembler_second_pass(Assembler *assembler, const char *output);

int assemble(Text *preprocessed, const char *output);

#endif //MIPS_ASSEMBLER_ASSEMBLER_H