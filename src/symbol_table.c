#include "symbol_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* SymbolTable

Uses a hash table to store declared symbols.
Symbols declarations are found and added in the assembler's first pass.
The second pass consults the symbol table when an instruction refers to a symbol.

Implemented as a hash table
*/

// Initializes and allocates memory for an empty symbol table
int st_init(SymbolTable *table) {
    table->buckets = malloc(SYMBOL_TABLE_SIZE * sizeof(SymbolBucket *));
    if (table->buckets == NULL) {
        raise_error(MEM, NULL, __FILE__);
        return 0;
    }

    table->size = 0;

    // Fill with null
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        table->buckets[i] = NULL;
    }

    return 1;
}

// Checks name using symbol.name
int st_add_struct(SymbolTable *table, const Symbol symbol) {
    unsigned long index = hash_key(symbol.name->str, SYMBOL_TABLE_SIZE);

    if (table->buckets[index] == NULL) {
        SymbolBucket *new = malloc(sizeof(SymbolBucket));
        if (new == NULL) {
            raise_error(MEM, NULL, __FILE__);
            return 0;
        }
        table->buckets[index] = new;
        table->buckets[index]->item = symbol;
        table->buckets[index]->next = NULL;
        table->size++;
    } else {
        SymbolBucket *prev = NULL;
        SymbolBucket *cur = table->buckets[index];
        while (cur != NULL) {
            // Symbol exists, update if undefined
            if (strcmp(cur->item.name->str, symbol.name->str) == 0) {
                if (cur->item.segment == UNDEF) {
                    cur->item.offset = symbol.offset;
                    cur->item.segment = symbol.segment;
                    string_destroy(symbol.name);
                    return 1;
                }
                raise_error(DUPL_DEF, symbol.name->str, __FILE__);
                return 0;
            }
            prev = cur;
            cur = cur->next;
        }
        // Reached end of linked list; create new entry after 'cur'
        SymbolBucket *new = malloc(sizeof(SymbolBucket));
        if (new == NULL) {
            raise_error(MEM, NULL, __FILE__);
            return 0;
        }
        if (prev != NULL) prev->next = new;
        new->item = symbol;
        new->next = NULL;
        table->size++;
    }

    return 1;
}

// Checks name using &strtab[symbol.strtab_index]
int st_add_by_name(SymbolTable *table, const char *strtab, Symbol symbol) {
    unsigned long index = hash_key(&strtab[symbol.strtab_index], SYMBOL_TABLE_SIZE);

    if (table->buckets[index] == NULL) {
        SymbolBucket *new = malloc(sizeof(SymbolBucket));
        if (new == NULL) {
            raise_error(MEM, NULL, __FILE__);
            return 0;
        }
        table->buckets[index] = new;
        table->buckets[index]->item = symbol;
        table->buckets[index]->next = NULL;
        table->size++;
    } else {
        SymbolBucket *prev = NULL;
        SymbolBucket *cur = table->buckets[index];
        while (cur != NULL) {
            // Symbol exists, update if undefined
            if (strcmp(&strtab[cur->item.strtab_index], &strtab[symbol.strtab_index]) == 0) {
                if (cur->item.segment == UNDEF) {
                    cur->item.offset = symbol.offset;
                    cur->item.segment = symbol.segment;
                    return 1;
                }
                raise_error(DUPL_DEF, &strtab[symbol.strtab_index], __FILE__);
                return 0;
            }
            prev = cur;
            cur = cur->next;
        }
        // Reached end of linked list; create new entry after 'cur'
        SymbolBucket *new = malloc(sizeof(SymbolBucket));
        if (new == NULL) {
            raise_error(MEM, NULL, __FILE__);
            return 0;
        }
        if (prev != NULL) prev->next = new;
        new->item = symbol;
        new->next = NULL;
        table->size++;
    }

    return 1;
}

// Adds to symbol table. Does not check if the symbol is valid per MIPS guidelines (i.e., alphanumeric only).
int st_add_symbol(SymbolTable *table, const char *name, const uint32_t offset, enum Segment segment, enum Binding binding) {
    Symbol s;
    s.name = malloc(sizeof(String));
    if (s.name == NULL) {
        raise_error(MEM, NULL, __FILE__);
        return 0;
    }
    if (string_init(s.name) == 0) return 0;
    string_cpy_to(s.name, name);
    s.offset = offset;
    s.segment = segment;
    s.binding = binding;
    s.strtab_index = -1;

    return st_add_struct(table, s);
}

// Returns the index of the linked list, or SYMBOL_TABLE_SIZE if not found
unsigned long st_exists(const SymbolTable *table, const char *name) {
    unsigned long index = hash_key(name, SYMBOL_TABLE_SIZE);
    if (table->buckets[index] == NULL) {
        return SYMBOL_TABLE_SIZE;
    }
    const SymbolBucket *cur = table->buckets[index];
    while (cur != NULL) {
        if (strcmp(cur->item.name->str, name) == 0) {
            return index;
        }
        cur = cur->next;
    }
    return SYMBOL_TABLE_SIZE;
}

unsigned long st_exists_by_name(const SymbolTable *table, const char *strtab, const char *name) {
    unsigned long index = hash_key(name, SYMBOL_TABLE_SIZE);
    if (table->buckets[index] == NULL) {
        return SYMBOL_TABLE_SIZE;
    }
    const SymbolBucket *cur = table->buckets[index];
    while (cur != NULL) {
        if (strcmp(&strtab[cur->item.strtab_index], name) == 0) {
            return index;
        }
        cur = cur->next;
    }
    return SYMBOL_TABLE_SIZE;
}

// Returns pointer to symbol with given name, or NULL on failure (does not raise error)
Symbol * st_get_symbol(const SymbolTable *table, const char *name) {
    unsigned long index = st_exists(table, name);
    if (index == SYMBOL_TABLE_SIZE) {
        return NULL;
    }

    SymbolBucket *cur = table->buckets[index];
    while (cur != NULL) {
        if (strcmp(cur->item.name->str, name) == 0) {
            return &cur->item;
        }
        cur = cur->next;
    }

    return NULL;
}

// Returns pointer to symbol with given name, or NULL on failure (does not raise error)
Symbol * st_get_symbol_by_name(const SymbolTable *table, const char *strtab, const char *name) {
    unsigned long index = st_exists_by_name(table, strtab, name);
    if (index == SYMBOL_TABLE_SIZE) {
        return NULL;
    }

    SymbolBucket *cur = table->buckets[index];
    while (cur != NULL) {
        if (strcmp(&strtab[cur->item.strtab_index], name) == 0) {
            return &cur->item;
        }
        cur = cur->next;
    }

    return NULL;
}

// Gets the symbol, but raises error on failure
Symbol * st_get_symbol_safe(const SymbolTable *table, const char *name) {
    Symbol *s = st_get_symbol(table, name);
    if (s == NULL) raise_error(TOKEN_ERR, name, __FILE__);
    return s;
}

// Recursively frees malloc'd buckets
void bucket_destroy(SymbolBucket *bucket) {
    if (bucket == NULL) return;
    if (bucket->item.name != NULL) free(bucket->item.name);
    if (bucket->next != NULL) bucket_destroy(bucket->next);
    free(bucket);
}

// Frees resources
void st_destroy(const SymbolTable *t) {
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        bucket_destroy(t->buckets[i]);
    }
    free(t->buckets);
}

void symbol_debug(const Symbol s) {
    if (s.segment == TEXT)
        printf("%s: .text + %d, binding %d\n", s.name->str, s.offset, s.binding);
    else if (s.segment == DATA)
        printf("%s: .data + %d, binding %d\n", s.name->str, s.offset, s.binding);
    else if (s.segment == UNDEF)
        printf("%s: undefined, binding %d\n", s.name->str, s.binding);
}

void st_debug(const SymbolTable *table) {
    for (int i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        SymbolBucket *cur = table->buckets[i];
        while (cur != NULL) {
            symbol_debug(cur->item);
            cur = cur->next;
        }
    }
}

int write_symbol_table(FILE *file, const SymbolTable *table) {
    write_word(file, table->size);
    for (size_t i = 0; i < SYMBOL_TABLE_SIZE; i++) {
        const SymbolBucket *cur = table->buckets[i];
        while (cur != NULL) {
            const Symbol s = cur->item;
            if (fwrite(&s.strtab_index, sizeof(s.strtab_index), 1, file) != 1) goto file_err;
            if (fwrite(&s.offset, sizeof(s.offset), 1, file) != 1) goto file_err;
            if (fwrite(&s.segment, sizeof(s.segment), 1, file) != 1) goto file_err;
            if (fwrite(&s.binding, sizeof(s.binding), 1, file) != 1) goto file_err;
            cur = cur->next;
            continue;

            file_err:
            ERROR_HANDLER.err_code = FILE_IO;
            return 0;
        }
    }
    return 1;
}