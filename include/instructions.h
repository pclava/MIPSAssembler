#ifndef MIPS_ASSEMBLER_INSTRUCTIONS_H
#define MIPS_ASSEMBLER_INSTRUCTIONS_H

#include "instruction_parser.h"
#include "symbol_table.h"
#include "reloc_table.h"

/* === TYPES === */

enum InstructionFormat {
    R,
    I,
    J
};

typedef struct {
    char *mnemonic;
    int opcode;
    int funct;
    enum InstructionFormat format;

    // What registers the given order of registers correspond to (rs=0,rt=1,rd=2,none=-1)
    // E.g., {1,0,-1} means "rt, rs"
    int register_order[3];

} InstrDesc;

typedef struct {
    InstrDesc item;
    unsigned char inUse;
} ITBucket;

// Fixed length hash table to store instruction descriptions
typedef struct {
    ITBucket *buckets;
    size_t size;
} InstructionTable;

/* === INSTRUCTIONTABLE METHODS === */

int it_init(InstructionTable *table);

int it_create(InstructionTable *table);

int it_insert(InstructionTable *table, InstrDesc desc);

InstrDesc *it_lookup(const InstructionTable *table, const char *mnemonic);

void it_destroy(const InstructionTable *table);

/* === INSTRUCTION CONVERSION === */
int get_registers(unsigned int *out, const unsigned char *in, const int *order);

uint32_t convert_rtype(Instruction instruction, const InstrDesc *desc);

uint32_t convert_itype(Instruction instruction, SymbolTable *symbol_table, RelocationTable *reloc_table, const InstrDesc *desc, uint32_t current_offset);

uint32_t convert_jtype(Instruction instruction, const SymbolTable *symbol_table, RelocationTable *reloc_table, const InstrDesc *desc, uint32_t current_offset);

#endif //MIPS_ASSEMBLER_INSTRUCTIONS_H