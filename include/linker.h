#ifndef MIPS_ASSEMBLER_LINKER_H
#define MIPS_ASSEMBLER_LINKER_H
#include <stdint.h>
#include "symbol_table.h"
#include "mof.h"

typedef struct SourceFile SourceFile;

struct SourceFile {
    char *name;
    uint32_t text_offset;
    uint32_t data_offset;
    uint32_t ktext_offset;
    uint32_t kdata_offset;
    SymbolTable *symbol_table;  // NOTE: global symbols store their final address in the offset field. segment is ignored
    struct mof_file file;
};

struct linker_settings {
    int clean;
    int link_start;
    char *dump_symbols;
    char *out_path;
    char *entry;
};

int link(char *object_files[], int file_count, struct linker_settings settings);

#endif //MIPS_ASSEMBLER_LINKER_H