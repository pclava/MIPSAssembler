#ifndef MIPS_ASSEMBLER_SYMBOL_TABLE_H
#define MIPS_ASSEMBLER_SYMBOL_TABLE_H
#include <stddef.h>
#include <stdint.h>
#include "utils.h"

#define SYMBOL_TABLE_SIZE 256

/* === TYPES === */

// Make packed to ensure constant size and offset between systems
typedef struct __attribute__((packed)) {
    char name[SYMBOL_SIZE]; // LABELS CAN BE UP TO 31 CHARACTERS
    uint32_t offset;        // Offset relative to start of section
    uint8_t segment;        // Text or data
    uint8_t binding;        // Local, global, or undefined
} Symbol;

typedef struct {
    Symbol item;
    void *next;
} SymbolBucket;

typedef struct {
    SymbolBucket **buckets;
    size_t size;
} SymbolTable;

/* === SYMBOL TABLE METHODS === */

int st_init(SymbolTable *table);

int st_add_struct(SymbolTable *table, Symbol symbol);

int st_add_symbol(SymbolTable *table, const char *name, uint32_t offset, enum Segment segment, enum Binding binding);

unsigned long st_exists(const SymbolTable *table, const char *name);

Symbol * st_get_symbol(const SymbolTable *table, const char *name);

Symbol * st_get_symbol_safe(const SymbolTable *table, const char *name);

void st_destroy(const SymbolTable *t);

void st_debug(const SymbolTable *t);

int write_symbol_table(FILE *file, const SymbolTable *t);

#endif //MIPS_ASSEMBLER_SYMBOL_TABLE_H