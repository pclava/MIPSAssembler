#include "strings.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

// note string is guaranteed to be filled with null bytes after str[len]

static int str_resize(String *str, size_t size) {
    str->cap = size;
    char *new_str = realloc(str->str, str->cap * sizeof(char));
    if (new_str == NULL) {
        raise_error(MEM, NULL, __FILE__);
        return 0;
    }
    str->str = new_str;
    memset(&str->str[str->len], '\0', str->cap - str->len); // fill the rest with null
    return 1;
}

int string_init(String *str) {
    char *s = malloc(STRING_INIT);
    if (s == NULL) {
        raise_error(MEM, NULL, __FILE__);
        return 0;
    }

    str->str = s;
    memset(str->str, '\0', STRING_INIT);
    str->cap = STRING_INIT;
    str->len = 0;
    str->offset = -1;

    return 1;
}

void string_destroy(String *str) {
    if (str == NULL) return;
    if (str->str != NULL) free(str->str);
}

int string_append(String *str, char c) {
    if (str->len + 1 >= str->cap) {
        if (str_resize(str, str->cap * 2) == 0) return 0;
    }

    str->str[str->len] = c;
    str->len++;
    return 1;
}

// Does not check bounds!
char string_get(const String *str, size_t index) {
    return str->str[index];
}

void string_insert(const String *str, size_t index, char c) {
    str->str[index] = c;
}

// Writes src to dst, resizing if necessary
int string_cpy_to(String *dst, const char *src) {
    const size_t l = strlen(src);
    if (dst->cap < l) {
        if (str_resize(dst, l+1) == 0) return 0;
    }
    strcpy(dst->str, src);
    dst->len = l;
    return 1;
}

void string_cpy_from(char *src, const String *dst) {
    strcpy(src, dst->str);
}

int string_write(String *str, FILE *file) {
    if (fwrite(str->str, sizeof(char), str->len+1, file) != str->len+1) {
        ERROR_HANDLER.err_code = FILE_IO;
        return 0;
    }
    return 1;
}

int strtab_init(StringTable *table) {
    table->table = malloc(SYMBOL_TABLE_SIZE * sizeof(String *));
    if (table->table == NULL) {
        raise_error(MEM, NULL, __FILE__);
        return 0;
    }
    table->cap = SYMBOL_TABLE_SIZE;
    table->len = 0;
    table->size = 0;
    return 1;
}

void strtab_destroy(StringTable *table) {
    if (table == NULL) return;
    if (table->table != NULL) free(table->table);
}

// Returns byte offset in final string table
size_t strtab_add(StringTable *table, String *str) {

    // Check size
    if (table->size >= table->cap) {
        table->cap *= 2;
        String **new = realloc(table->table, table->cap * sizeof(String *));
        if (new == NULL) {
            raise_error(MEM, NULL, __FILE__);
            return 0;
        }
        table->table = new;
    }

    table->table[table->len] = str;
    table->len++;
    const size_t old = table->size;
    table->size += str->len+1;
    return old;
}

void strtab_populate(StringTable *table, SymbolTable *symbol_table) {
    // Loop through symbol table
    for (size_t i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        SymbolBucket *cur = symbol_table->buckets[i];
        while (cur != NULL) {
            uint32_t offset = strtab_add(table, cur->item.name);
            cur->item.strtab_index = offset;
            cur->item.name->offset = offset;
            cur = cur->next;
        }
    }
}

int write_string_table(FILE *file, const StringTable *table) {
    write_word(file, table->size);
    for (size_t i = 0; i < table->len; i++) {
        // Write string to file
        if (string_write(table->table[i], file) == 0) return 0;
    }
    return 1;
}

void strtab_debug(const StringTable *table) {
    size_t j = 0;
    for (size_t i = 0; i < table->len; i++) {
        printf("0x%.2lx: %s\n", j, table->table[i]->str);
        j+=table->table[i]->len+1;
    }
}