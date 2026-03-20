#ifndef MIPS_ASSEMBLER_STRINGS_H
#define MIPS_ASSEMBLER_STRINGS_H
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define STRING_INIT 32

typedef struct String String;
typedef struct StringTable StringTable;

struct String {
    char *str;
    size_t cap;
    size_t len;         // result of strlen() (i.e., does not count null terminator)
};

struct StringTable {
    char *table;
    size_t cap;
    uint32_t size;    // size in bytes
};

int string_init(String *str);

void string_destroy(String *str);

int string_append(String *str, char c);

int string_append_string(String *str, const char *c);

char string_get(const String *str, size_t index);

void string_set(const String *str, size_t index, char c);

int string_insert(String *str, size_t index, const char *src);

int string_cpy_to(String *dst, const char *src);

void string_cpy_from(char *dst, const String *src);

char *strtab_get(const StringTable *table, uint32_t index);

int strtab_init(StringTable *table);

void strtab_destroy(StringTable *table);

uint32_t strtab_add(StringTable *table, const char *str);

int write_string_table(FILE *file, const StringTable *table);

void strtab_debug(const StringTable *table);

void strtab_debug2(const char *strtab, uint32_t size);

#endif //MIPS_ASSEMBLER_STRINGS_H