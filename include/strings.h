#ifndef MIPS_ASSEMBLER_STRINGS_H
#define MIPS_ASSEMBLER_STRINGS_H
#include <stddef.h>
#include <stdio.h>

#include "symbol_table.h"

#define STRING_INIT 32

typedef struct String String;
typedef struct StringTable StringTable;

// Forward declaration
typedef struct SymbolTable SymbolTable;

struct String {
    char *str;
    size_t cap;
    size_t len;
    uint32_t offset; // index in string table
};

struct StringTable {
    String **table;
    size_t cap;
    size_t len;     // number of entries
    size_t size;    // size in bytes
};

int string_init(String *str);

void string_destroy(String *str);

int string_append(String *str, char c);

char string_get(const String *str, size_t index);

void string_insert(const String *str, size_t index, char c);

int string_cpy_to(String *dst, const char *src);

void string_cpy_from(char *dst, const String *src);

int strtab_init(StringTable *table);

void strtab_destroy(StringTable *table);

size_t strtab_add(StringTable *table, String *s);

void strtab_populate(StringTable *table, SymbolTable *symbol_table);

int write_string_table(FILE *file, const StringTable *table);

void strtab_debug(const StringTable *table);

#endif //MIPS_ASSEMBLER_STRINGS_H