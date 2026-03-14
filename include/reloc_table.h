#ifndef MIPS_ASSEMBLER_RELOC_TABLE_H
#define MIPS_ASSEMBLER_RELOC_TABLE_H
#include <stdint.h>
#include "utils.h"
#include "mof.h"

/* === TYPES === */

typedef struct RelocationTable RelocationTable;
typedef struct mof_relocation RelocationEntry;  // dont touch this

struct RelocationTable {
    RelocationEntry *list;
    size_t len;
    size_t cap;
};

/* === RELOCTABLE METHODS === */

int rt_init(RelocationTable *table);

int re_init(RelocationEntry *reloc, uint32_t index, uint32_t target_offset, Segment segment, RelocType reloc_type);

int rt_add(RelocationTable *table, RelocationEntry entry);

void rt_destroy(const RelocationTable *table);

void rt_debug(const RelocationTable *table, const SymbolTable *symbol_table);

void re_debug(RelocationEntry entry, const char *dependency);

int write_reloc_table(FILE *file, const RelocationTable *table);

#endif //MIPS_ASSEMBLER_RELOC_TABLE_H