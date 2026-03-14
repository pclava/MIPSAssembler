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
    SymbolTable *symbol_table;  // NOTE: global symbols store their final address in the offset field. segment is ignored
    struct mof_file file;
};

int link(const char *out_path, char *object_files[], int file_count, const char *entry_symbol);

#endif //MIPS_ASSEMBLER_LINKER_H