#ifndef MIPS_ASSEMBLER_PSEUDOINSTRUCTIONS_H
#define MIPS_ASSEMBLER_PSEUDOINSTRUCTIONS_H
#include "instruction_parser.h"

#define MACRO_TABLE_LENGTH 256
#define MACRO_SIZE 32

typedef struct Macro Macro;
typedef struct MacroBucket MacroBucket;
typedef struct MacroTable MacroTable;

enum MacroType {
    FUNCTION,       // .macro directive
    CONSTANT,       // .eqv and .define directives
};

struct Macro {
    char name[MACRO_SIZE];
    Line *definition;
    size_t definition_length;
    char args[3][32];
    enum MacroType type; // macro (0) or symbolic constant (1)
};

struct MacroBucket {
    Macro macro;
    unsigned char inUse;
};

struct MacroTable {
    MacroBucket *buckets;
    size_t size;
};

int mt_init(MacroTable *table);

int mt_add(MacroTable *table, Macro macro);

unsigned long mt_exists(const MacroTable *table, const char *name);

Macro *mt_get(const MacroTable *table, const char *name);

Macro * mt_get_at(const MacroTable *table, unsigned long index);

void mt_destroy(const MacroTable *t);

void mt_debug(const MacroTable *t);

char *macro_get_constant(const Macro *macro);

void macro_debug(const Macro *m);

int define_constant(Macro *macro, const Line *line);

Line *define_macro(Macro *macro, const Line *line);

int insert_macro(Text *text_list, Macro *macro, Line *line);

int li(Instruction, InstructionList*);

#endif //MIPS_ASSEMBLER_PSEUDOINSTRUCTIONS_H