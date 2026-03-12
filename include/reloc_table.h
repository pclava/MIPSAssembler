#ifndef MIPS_ASSEMBLER_RELOC_TABLE_H
#define MIPS_ASSEMBLER_RELOC_TABLE_H
#include <stdint.h>
#include "utils.h"

/* === TYPES === */

typedef struct RelocationEntry RelocationEntry;
typedef struct RelocationTable RelocationTable;

#define RE_SIZE 10
struct RelocationEntry {
    // "address at (segment+target_offset) requires relocation of type (reloc_type) for the symbol (dependency)"
    uint32_t target_offset;     // Instruction or data item that depends on relocation
    uint32_t strtab_index;      // Byte offset in string table
    String *dependency;         // Symbol the target depends on
    uint8_t segment;            // Segment (text or data). This is needed so the linker can determine the absolute address
    uint8_t reloc_type;         // Type of relocation needed
};

struct RelocationTable {
    RelocationEntry *list;
    size_t len;
    size_t cap;
};

/* === RELOCTABLE METHODS === */

int rt_init(RelocationTable *table);

int re_init(RelocationEntry *reloc, uint32_t offset, enum Segment segment, enum RelocType reloc_type, String *dependency);

int rt_add(RelocationTable *table, RelocationEntry entry);

void rt_destroy(const RelocationTable *table);

void rt_debug(const RelocationTable *table);

void re_debug(RelocationEntry);

int write_reloc_table(FILE *file, const RelocationTable *table);

#endif //MIPS_ASSEMBLER_RELOC_TABLE_H