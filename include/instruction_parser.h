#ifndef MIPS_ASSEMBLER_INSTRUCTION_PARSER_H
#define MIPS_ASSEMBLER_INSTRUCTION_PARSER_H
#include <stdint.h>
#include "utils.h"

typedef struct Instruction Instruction;
typedef struct InstructionList InstructionList;

struct Instruction {
    char mnemonic[MNEMONIC_LENGTH]; // longest mnemonic is 5 or 6 characters, including some extra bytes just in case
    unsigned char registers[3];
    Immediate imm;
    const Line *line; // corresponding line in the Text list
} ;

struct InstructionList {
    size_t len;
    size_t cap;
    uint32_t text_offset; // size of text segment in bytes
    Instruction *list;
};

/* === INSTRUCTIONLIST METHODS === */

int il_init(InstructionList * instruction_list, uint32_t entry);

int add_instruction(InstructionList * instruction_list, Instruction instr);

void il_destroy(const InstructionList * instruction_list);

void il_debug(const InstructionList *);

void instruction_debug(Instruction);

#endif //MIPS_ASSEMBLER_INSTRUCTION_PARSER_H