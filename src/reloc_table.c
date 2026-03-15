#include "reloc_table.h"
#include "symbol_table.h"
#include "strings.h"
#include <stdlib.h>
#include <string.h>

/* Relocation table

These handle the RelocationTable, which keeps track of instructions and data items
needing relocation after linking.
*/

// Initializes empty RelocationTable
int rt_init(RelocationTable *table) {
    table->len = 0;
    table->cap = 64;
    table->list = malloc(table->cap * sizeof(RelocationEntry));
    if (table->list == NULL) {
        raise_error(MEM, NULL, __FILE__);
        return 0;
    }
    return 1;
}

// Initializes a RelocationEntry
int re_init(RelocationEntry *reloc, uint32_t index, uint32_t offset, Segment segment, RelocType type) {
    reloc->offset = offset;
    reloc->type = type;
    reloc->segment = segment;
    reloc->index = index;
    return 1;
}

// Adds the relocation entry
int rt_add(RelocationTable *table, RelocationEntry entry) {
    if (table->len >= table->cap) {
        table->cap *= 2;
        RelocationEntry *new = realloc(table->list, table->cap * sizeof(RelocationEntry));
        if (new == NULL) return 0;
        table->list = new;
    }
    table->list[table->len] = entry;
    table->len++;
    return 1;
}

// Frees resources
void rt_destroy(const RelocationTable *table) {
    free(table->list);
}

void rt_debug(const RelocationTable *table, const SymbolTable *symbol_table) {
    for (size_t i = 0; i < table->len; i++) {
        const char *name = strtab_get(symbol_table->string_table, table->list[i].index);
        re_debug(table->list[i], name);
    }
}

void re_debug(const RelocationEntry entry, const char *dependency) {
    char segment[6];
    if (entry.segment == TEXT) {
        strcpy(segment, ".text");
    } else {
        strcpy(segment, ".data");
    }

    printf("address at %s+%d needs relocation of type %d for symbol %s\n", segment, entry.offset, entry.type, dependency);
}

int write_reloc_table(FILE *file, const RelocationTable *table) {
    for (size_t i = 0; i < table->len; i++) {
        const struct mof_relocation entry = table->list[i];

        if (mof_write_relocation(file, &entry) == 0) {
            ERROR_HANDLER.err_code = FILE_IO;
            return 0;
        }
    }
    return 1;
}