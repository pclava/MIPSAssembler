#ifndef MIPS_ASSEMBLER_SYMBOL_TABLE_H
#define MIPS_ASSEMBLER_SYMBOL_TABLE_H
#include <stdint.h>

#include "utils.h"
#include "mof.h"

#define SYMBOL_TABLE_SIZE 256

/* === TYPES === */

typedef struct SymbolBucket SymbolBucket;
typedef struct SymbolTable SymbolTable;

// Forward declaration
typedef struct StringTable StringTable;
typedef enum mof_binding Binding;
typedef enum mof_segment Segment;

typedef struct mof_symbol Symbol;   // dont touch this

struct SymbolBucket {
    Symbol item;
    void *next;
};

struct SymbolTable {
    SymbolBucket **buckets;
    StringTable *string_table;
    size_t size;
};

/* === SYMBOL TABLE METHODS === */

int st_init(SymbolTable *table);

int st_add_struct(SymbolTable *table, Symbol symbol, const char *name);

int st_add_symbol(SymbolTable *table, const char *name, uint32_t offset, Segment segment, Binding binding);

char * st_get_string(const SymbolTable *table, Symbol symbol);

unsigned long st_exists(const SymbolTable *table, const char *name);

Symbol * st_get_symbol(const SymbolTable *table, const char *name);

Symbol * st_get_symbol_safe(const SymbolTable *table, const char *name);

void st_destroy(const SymbolTable *t);

void st_debug(const SymbolTable *t);

int write_symbol_table(FILE *file, const SymbolTable *t);

#endif //MIPS_ASSEMBLER_SYMBOL_TABLE_H