#ifndef MIPS_ASSEMBLER_SYMBOL_TABLE_H
#define MIPS_ASSEMBLER_SYMBOL_TABLE_H
#include <stdint.h>

#include "strings.h"
#include "utils.h"

#define SYMBOL_TABLE_SIZE 256

/* === TYPES === */

typedef struct Symbol Symbol;
typedef struct SymbolBucket SymbolBucket;
typedef struct SymbolTable SymbolTable;

// Forward declaration
typedef struct String String;
enum Segment;
enum Binding;

#define SYMBOL_SIZE 10
struct Symbol {
    String *name;
    uint32_t offset;        // Offset relative to start of section
    uint32_t strtab_index;  // Byte offset in string table
    uint8_t segment;        // Text or data
    uint8_t binding;        // Local, global, or undefined
};

struct SymbolBucket {
    Symbol item;
    void *next;
};

struct SymbolTable {
    SymbolBucket **buckets;
    size_t size;
};

/* === SYMBOL TABLE METHODS === */

int st_init(SymbolTable *table);

int st_add_struct(SymbolTable *table, Symbol symbol);

int st_add_by_name(SymbolTable *table, const char *strtab, Symbol symbol);

int st_add_symbol(SymbolTable *table, const char *name, uint32_t offset, enum Segment segment, enum Binding binding);

unsigned long st_exists(const SymbolTable *table, const char *name);

unsigned long st_exists_by_name(const SymbolTable *table, const char *strtab, const char *name);

Symbol * st_get_symbol(const SymbolTable *table, const char *name);

Symbol * st_get_symbol_safe(const SymbolTable *table, const char *name);

Symbol * st_get_symbol_by_name(const SymbolTable *table, const char *strtab, const char *name);

void st_destroy(const SymbolTable *t);

void st_debug(const SymbolTable *t);

int write_symbol_table(FILE *file, const SymbolTable *t);

#endif //MIPS_ASSEMBLER_SYMBOL_TABLE_H