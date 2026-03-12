#ifndef MIPS_ASSEMBLER_LINKER_H
#define MIPS_ASSEMBLER_LINKER_H
#include <stdint.h>
#include "symbol_table.h"
#include "reloc_table.h"

typedef struct SourceFile SourceFile;

struct SourceFile {
    uint32_t text_offset;
    uint32_t data_offset;
    uint32_t text_size;
    uint32_t data_size;
    uint32_t *text;
    uint8_t *data;
    SymbolTable *symbol_table;
    RelocationTable *relocation_table;
    char *string_table;
    char *name;
};

int link(const char *out_path, char *object_files[], int file_count, const char *entry_symbol);

#endif //MIPS_ASSEMBLER_LINKER_H