#include "instruction_parser.h"

#include <stdlib.h>

/* Instruction parser

These functions handle the InstructionList, where instructions are stored after
being parsed by the assembler's first passed.
*/

// Initializes an InstructionList with addresses beginning at 'entry'
int il_init(InstructionList * instruction_list, uint32_t entry) {
    instruction_list->len = 0;
    instruction_list->cap = 64;
    instruction_list->list = malloc(instruction_list->cap * sizeof(Instruction));
    if (instruction_list->list == NULL) {
        raise_error(MEM, NULL, __FILE__);
        return 0;
    }
    instruction_list->text_offset = entry;
    return 1;
}

// Adds the instruction and increments addr
int add_instruction(InstructionList *instruction_list, Instruction instr) {
    if (instruction_list->len >= instruction_list->cap) {
        instruction_list->cap = instruction_list->cap * 2;
        Instruction *new = realloc(instruction_list->list, instruction_list->cap * sizeof(Instruction));
        if (new == NULL) {
            return 0;
        }
        instruction_list->list = new;
    }

    instruction_list->list[instruction_list->len] = instr;
    instruction_list->len++;
    instruction_list->text_offset += 4;
    return 1;
}

// Frees resources
void il_destroy(const InstructionList *instruction_list) {
    free(instruction_list->list);
}

void il_debug(const InstructionList *instruction_list) {
    for (size_t i = 0; i < instruction_list->len; i++) {
        instruction_debug(instruction_list->list[i]);
    }
}

void instruction_debug(const Instruction instruction) {
    printf("instruction: %s (%d, %d, %d)", instruction.mnemonic, instruction.registers[0], instruction.registers[1], instruction.registers[2]);
    if (instruction.imm.type == NUM) {
        printf(" imm: 0x%x\n", instruction.imm.intValue);
    } else if (instruction.imm.type == SYMBOL) {
        printf(" imm: symbol 0x%.2x\n", instruction.imm.symbol->offset);
    }
    else {
        printf("\n");
    }
}
