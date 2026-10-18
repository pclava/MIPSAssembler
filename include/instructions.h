#ifndef MIPS_ASSEMBLER_INSTRUCTIONS_H
#define MIPS_ASSEMBLER_INSTRUCTIONS_H

#include <stdbool.h>

#include "instruction_parser.h"
#include "symbol_table.h"
#include "reloc_table.h"

/* === TYPES === */

typedef struct InstrDesc InstrDesc;
typedef struct ITBucket ITBucket;
typedef struct InstructionTable InstructionTable;

typedef enum ArgPosition ArgPosition;
typedef struct ArgValue ArgValue;

enum ArgPosition {
    NOARG,

    // Register
    RS,
    RT,
    RD,

    // Immediate
    SHAMT,
    RDIMM,
    JADDR,
    IMM16,
    PC16,
    REGOFF,
    SEL,
};

// Pre-determined values (usually rs) (note: default value of zero does not need to be specified)
struct ArgValue {
    ArgPosition position;
    int value;
};

struct InstrDesc {
    char *mnemonic;
    int argc;

    int opcode;
    ArgPosition args[ARGCOUNT];
    ArgValue defaults[5];
    int funct;                  // used by some instructions, -1 otherwise
};

struct ITBucket {
    InstrDesc item;
    unsigned char inUse;
};

// Fixed length hash table to store instruction descriptions
struct InstructionTable {
    ITBucket *buckets;
    size_t size;
};

/* === INSTRUCTIONTABLE METHODS === */

int it_init(InstructionTable *table);

int it_create(InstructionTable *table);

int it_insert(InstructionTable *table, InstrDesc desc);

InstrDesc *it_lookup(const InstructionTable *table, const char *mnemonic, int argc);

void it_destroy(const InstructionTable *table);

/* === INSTRUCTION CONVERSION === */
uint32_t convert_instruction(Instruction instruction, InstructionTable *instruction_table, RelocationTable *reloc_table, uint32_t current_addr, Segment current_segment);

uint32_t convert_rtype(Instruction instruction, const InstrDesc *desc);

uint32_t convert_itype(Instruction instruction, RelocationTable *reloc_table, const InstrDesc *desc, uint32_t current_offset, Segment current_segment);

uint32_t convert_jtype(Instruction instruction, RelocationTable *reloc_table, const InstrDesc *desc, uint32_t current_offset, Segment current_segment);

#endif //MIPS_ASSEMBLER_INSTRUCTIONS_H