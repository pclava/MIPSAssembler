#include "reloc_table.h"
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
int re_init(RelocationEntry *reloc, uint32_t offset, enum Segment segment, enum RelocType reloc_type, String *dependency) {
    reloc->target_offset = offset;
    reloc->reloc_type = reloc_type;
    reloc->segment = segment;
    reloc->dependency = dependency;
    reloc->strtab_index = -1;
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

void rt_debug(const RelocationTable *table) {
    for (size_t i = 0; i < table->len; i++) {
        re_debug(table->list[i]);
    }
}

void re_debug(const RelocationEntry entry) {
    char segment[6];
    if (entry.segment == TEXT) {
        strcpy(segment, ".text");
    } else {
        strcpy(segment, ".data");
    }

    printf("address at %s+%d needs relocation of type %d for symbol %s\n", segment, entry.target_offset, entry.reloc_type, entry.dependency->str);
}

int write_reloc_table(FILE *file, const RelocationTable *table) {
    write_word(file, table->len);
    for (size_t i = 0; i < table->len; i++) {
        const RelocationEntry *entry = &table->list[i];

        if (fwrite(&entry->dependency->offset, sizeof(entry->dependency->offset), 1, file) != 1) goto file_err;
        if (fwrite(&entry->target_offset, sizeof(entry->target_offset), 1, file) != 1) goto file_err;
        if (fwrite(&entry->segment, sizeof(entry->segment), 1, file) != 1) goto file_err;
        if (fwrite(&entry->reloc_type, sizeof(entry->reloc_type), 1, file) != 1) goto file_err;
        continue;

        file_err:
        ERROR_HANDLER.err_code = FILE_IO;
        return 0;
    }
    return 1;
}