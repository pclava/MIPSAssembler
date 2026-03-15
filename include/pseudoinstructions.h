#ifndef MIPS_ASSEMBLER_PSEUDOINSTRUCTIONS_H
#define MIPS_ASSEMBLER_PSEUDOINSTRUCTIONS_H
#include "instruction_parser.h"

#define MACRO_TABLE_LENGTH 256
#define MACRO_SIZE 32

typedef struct Macro Macro;
typedef struct MacroBucket MacroBucket;
typedef struct MacroTable MacroTable;

struct Macro {
    char name[MACRO_SIZE];
    Line *definition_start;
    size_t definition_length;
    char args[32][32];
};

struct MacroBucket {
    Macro macro;
    unsigned char inUse;
};

struct MacroTable{
    MacroBucket *buckets;
    size_t size;
};

int mt_init(MacroTable *table);

int mt_add(MacroTable *table, Macro macro);

unsigned long mt_exists(const MacroTable *table, const char *name);

Macro *mt_get(const MacroTable *table, const char *name);

void mt_destroy(const MacroTable *t);

void mt_debug(const MacroTable *t);

void macro_debug(const Macro *m);

Line *define_macro(Macro *macro, const Line *line);

int insert_macro(Text *text_list, const MacroTable *table, const char *name, Line *line);

int li(Instruction, InstructionList*);
// int la(Instruction, InstructionList*);

#endif //MIPS_ASSEMBLER_PSEUDOINSTRUCTIONS_H