#include "strings.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

// note: string is guaranteed to be filled with null bytes after str[len]

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
    str->cap = STRING_INIT;
    str->len = 0;
    string_clear(str);

    return 1;
}

void string_destroy(String *str) {
    if (str == NULL) return;
    if (str->str != NULL) free(str->str);
}

int string_append(String *str, char c) {
    if (str->len + 1 >= str->cap) {
        try(str_resize(str, str->cap * 2), 0);
    }

    string_set(str, str->len, c);
    str->len++;
    return 1;
}

int string_append_string(String *str, const char *c) {
    return string_insert(str, str->len, c);
}

void string_clear(const String *str) {
    if (str == NULL) return;
    memset(str->str, '\0', str->cap);
}

// Does not check bounds!
char string_get(const String *str, size_t index) {
    return str->str[index];
}

void string_set(const String *str, size_t index, char c) {
    str->str[index] = c;
}

int string_insert(String *str, size_t index, const char *src) {
    size_t len = strlen(src);
    if (str->len + len >= str->cap) {
        try(str_resize(str, str->cap * 2 + len), 0)
    }

    memmove(&str->str[index+len], &str->str[index], str->len+1-index);
    memcpy(&str->str[index], src, len);
    str->len += len;
    return 1;
}

int string_insert_char(String *str, size_t index, char c) {
    if (str->len + 1 >= str->cap) {
        try(str_resize(str, str->cap * 2 + 1), 0)
    }

    memmove(&str->str[index+1], &str->str[index], str->len+1-index);
    str->str[index] = c;
    str->len += 1;
    return 1;
}

// Writes src to dst, resizing if necessary
int string_cpy_to(String *dst, const char *src) {
    const size_t l = strlen(src);
    if (dst->cap < l) {
        try(str_resize(dst, l+2), 0);
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

char *strtab_get(const StringTable *table, uint32_t index) {
    return &table->table[index];
}

int strtab_init(StringTable *table) {
    table->table = malloc(SYMBOL_TABLE_SIZE * sizeof(char));
    if (table->table == NULL) {
        raise_error(MEM, NULL, __FILE__);
        return 0;
    }
    table->cap = SYMBOL_TABLE_SIZE;
    table->size = 0;
    return 1;
}

void strtab_destroy(StringTable *table) {
    if (table == NULL) return;
    if (table->table != NULL) free(table->table);
}

// Returns byte offset in final string table
uint32_t strtab_add(StringTable *table, const char *str) {
    uint32_t len = (uint32_t) strlen(str);
    if (table->size + len + 1 > table->cap) {
        table->cap = table->size+len+1 > 2*table->cap ? table->size+len+1 : 2*table->cap;
        char *new = realloc(table->table, table->cap * sizeof(char));
        if (new == NULL) {
            raise_error(MEM, NULL, __FILE__);
            return 0;
        }
        table->table = new;
        memset(&table->table[table->size], '\0', table->cap - table->size); // zero out rest of table
    }

    const uint32_t old = table->size;
    strcpy(&table->table[old], str);
    table->size += len+1;
    return old;
}

int write_string_table(FILE *file, const StringTable *table) {
    if (fwrite(table->table, sizeof(char), table->size, file) != table->size) {
        ERROR_HANDLER.err_code = FILE_IO;
        return 0;
    }
    return 1;
}

void strtab_debug(const StringTable *table) {
    for (uint32_t i = 0; i < table->size; i++) {
        if (table->table[i] == '\0') printf("\n0x%.2x: ", i+1);
        else {
            if (i == 0) printf("0x00: ");
            printf("%c", table->table[i]);
        }
    }
    printf("\n");
}

void strtab_debug2(const char *strtab, uint32_t size) {
    for (uint32_t i = 0; i < size; i++) {
        if (strtab[i] == '\0') printf("\n0x%.2x: ", i+1);
        else {
            if (i == 0) printf("0x00: ");
            printf("%c", strtab[i]);
        }
    }
}