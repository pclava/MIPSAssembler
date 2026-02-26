#ifndef MIPS_ASSEMBLER_RELOC_TABLE_H
#define MIPS_ASSEMBLER_RELOC_TABLE_H
#include <stdint.h>
#include "utils.h"

/* === TYPES === */

typedef struct {
    // "address at (segment+target_offset) requires relocation of type (reloc_type) for the symbol (dependency)"
    uint32_t target_offset;       // Instruction or data item that depends on relocation
    enum Segment segment;         // Segment (text or data). This is needed so the linker can determine the absolute address
    enum RelocType reloc_type;    // Type of relocation needed
    char dependency[SYMBOL_SIZE]; // Symbol the target depends on
} RelocationEntry;

typedef struct {
    RelocationEntry *list;
    size_t len;
    size_t cap;
} RelocationTable;

/* === RELOCTABLE METHODS === */

int rt_init(RelocationTable *table);

int re_init(RelocationEntry *reloc, uint32_t offset, enum Segment segment, enum RelocType reloc_type, const char *dependency);

int rt_add(RelocationTable *table, RelocationEntry entry);

void rt_destroy(const RelocationTable *table);

void rt_debug(const RelocationTable *table);

void re_debug(RelocationEntry);

int write_reloc_table(FILE *file, const RelocationTable *table);

#endif //MIPS_ASSEMBLER_RELOC_TABLE_H