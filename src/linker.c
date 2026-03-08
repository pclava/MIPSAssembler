#include "linker.h"
#include "utils.h"
#include <stdlib.h>
#include <string.h>

void file_destroy(const SourceFile *file) {
    free(file->relocation_table);
    free(file->symbol_table);
}

int file_init(SourceFile *file, uint32_t text_offset, uint32_t data_offset, uint32_t text_size, uint32_t data_size) {
    file->text_offset = text_offset;
    file->data_offset = data_offset;
    file->text_size = text_size;
    file->data_size = data_size;
    uint32_t *text = malloc(text_size);
    if (text == NULL) {
        goto _init_failure;
    }
    file->text = text;
    uint8_t *data = malloc(data_size);
    if (data == NULL) {
        free(text);
        goto _init_failure;
    }
    file->data = data;

    RelocationTable *table = malloc(sizeof(RelocationTable));
    if (table == NULL) {
        free(text);
        free(data);
        goto _init_failure;
    }
    rt_init(table);
    file->relocation_table = table;

    SymbolTable *symbol_table = malloc(sizeof(SymbolTable));
    if (symbol_table == NULL) {
        free(text);
        free(data);
        free(table);
        goto _init_failure;
    }
    st_init(symbol_table);
    file->symbol_table = symbol_table;

    return 1;

    _init_failure:
    raise_error(MEM, NULL, __FILE__);
    return 0;

}

// Returns final memory address of a symbol, with the formula *section base + object file offset + symbol offset*
uint32_t get_final_address(const Symbol symbol, const uint32_t object_offset) {
    switch (symbol.segment) {
        case TEXT:
            return TEXT_START + object_offset + symbol.offset;
        case DATA:
            return DATA_START + object_offset + symbol.offset;
        default:
            return 0;
    }
}

int relocate(SourceFile file, RelocationEntry entry, uint32_t final_address) {
    if (entry.segment == UNDEF) return 0;
    const uint32_t instr_addr = TEXT_START + file.text_offset + entry.target_offset;
    const uint32_t instr_offset = entry.target_offset/4;
    switch (entry.reloc_type) {
        case R_32:
            // Check segment
            if (entry.segment != DATA) {
                fprintf(stderr, "Error linking %s: attempted R_32 relocation outside data segment\n", file.name);
                return 0;
            }
            // Replace bytes little-endian
            file.data[entry.target_offset] = final_address & 0xFF;
            file.data[entry.target_offset+1] = final_address >> 8 & 0xFF;
            file.data[entry.target_offset+2] = final_address >> 16 & 0xFF;
            file.data[entry.target_offset+3] = final_address >> 24 & 0xFF;
            return 1;
        case R_26:
            // Check segment
            if (entry.segment != TEXT) {
                fprintf(stderr, "Error linking %s: attempted R_26 relocation outside text segment\n", file.name);
                return 0;
            }
            // Check range (compare MSBs of instruction's real address and final address)
            if ((instr_addr & 0xF0000000) != (final_address & 0xF0000000)) {
                fprintf(stderr, "Error linking %s: jump target out of range\n", file.name);
                return 0;
            }
            // Replace 26 lower bits
            file.text[instr_offset] |= (final_address & 0x0FFFFFFFF) >> 2;
            return 1;
        case R_PC16:
            // Check segment
            if (entry.segment != TEXT) {
                fprintf(stderr, "Error linking %s: attempted R_PC16 relocation outside text segment\n", file.name);
                return 0;
            }
            // Check range (within 2^15 instructions)
            const int32_t dist = ((int32_t) final_address - ((int32_t) instr_addr + 4))/4;
            if (dist < INT16_MIN || dist > INT16_MAX) {
                fprintf(stderr, "Error linking %s: branch target out of range\n", file.name);
                return 0;
            }
            file.text[instr_offset] |= (uint16_t) dist;
            return 1;
        case R_HI16:
            if (entry.segment != TEXT) {
                fprintf(stderr, "Error linking %s: attempted R_HI16 relocation outside text segment\n", file.name);
                return 0;
            }
            file.text[instr_offset] |= final_address >> 16;
            return 1;
        case R_LO16:
            if (entry.segment != TEXT) {
                fprintf(stderr, "Error linking %s: attempted R_LO16 relocation outside text segment\n", file.name);
                return 0;
            }
            file.text[instr_offset] |= final_address & 0x0000FFFF;
            return 1;
        default:
            fprintf(stderr, "Error linking %s: unrecognized relocation directive\n", file.name);
            return 0;
    }
}

void debug_section(const SourceFile file, const enum Segment segment) {
    if (segment == TEXT) {
        for (uint32_t i = 0; i < file.text_size/4; i++) {
            printf("instruction %u: 0x%.8x\n", i, file.text[i]);
        }
    }
}

void load_file(FILE *source, const SourceFile *file, const struct FileHeader * header, SymbolTable *global_symbols) {
    // Read text
    for (uint32_t i = 0; i < header->text_size/4; i++) {
        file->text[i] = read_word(source);
    }

    // Read data
    for (uint32_t i = 0; i < header->data_size; i++) {
        file->data[i] = read_byte(source);
    }

    // Read relocation table
    const uint32_t reloc_size = read_word(source);
    for (uint32_t i = 0; i < reloc_size; i++) {
        RelocationEntry entry;
        fread(&entry, sizeof(RelocationEntry), 1, source);
        rt_add(file->relocation_table, entry);
    }

    // Read symbol table
    const uint32_t symbol_table_size = read_word(source);
    for (uint32_t i = 0; i < symbol_table_size; i++) {
        Symbol symbol;
        fread(&symbol, sizeof(Symbol), 1, source);
        st_add_struct(file->symbol_table, symbol);

        // Add global symbols to symbol table
        if (symbol.binding == GLOBAL && symbol.segment != UNDEF) {
            uint32_t final_address = 0;
            if (symbol.segment == TEXT) {
                final_address = get_final_address(symbol, file->text_offset);
            } else if (symbol.segment == DATA) {
                final_address = get_final_address(symbol, file->data_offset);
            }
            st_add_symbol(global_symbols, symbol.name, final_address, symbol.segment, GLOBAL);
        }
    }
}

int file_relocation(const SourceFile *source, const SymbolTable *global_symbols) {
    const RelocationTable *reloc_table = source->relocation_table;
    const uint32_t text_offset = source->text_offset;
    const uint32_t data_offset = source->data_offset;
    for (size_t i = 0; i < reloc_table->len; i++) {
        const RelocationEntry entry = reloc_table->list[i];
        const Symbol *dependency = st_get_symbol_safe(source->symbol_table, entry.dependency);

        // Get final address for each symbol
        uint32_t final_address = 0;
        switch (dependency->segment) {
            case TEXT:
                final_address = get_final_address(*dependency, text_offset);
                break;
            case DATA:
                final_address = get_final_address(*dependency, data_offset);
                break;
            case UNDEF:
                if (dependency->binding != GLOBAL) {
                    fprintf(stderr, "Error linking %s: symbol undefined\n", source->name);
                    return 0;
                }
                dependency = st_get_symbol_safe(global_symbols, entry.dependency);
                if (dependency == NULL) {
                    if (strcmp(entry.dependency, "main") == 0) {
                        fprintf(stderr, "Could not find symbol 'main'. Have you exported it with .globl?\n");
                    }
                    return 0;
                }
                final_address = dependency->offset;
                break;
            default:
                return 0;
        }

        // Resolve relocation
        if (relocate(*source, entry, final_address) == 0) {
            fprintf(stderr, "Error linking %s: relocation failed\n", source->name);
            return 0;
        }
    }
    return 1;
}

FILE * open_object_file(const char *path, struct FileHeader *final_header, struct FileHeader *header) {
    FILE *f = fopen(path, "rb");
    if (f == NULL) {
        raise_error(FILE_IO, path, __FILE__);
        return NULL;
    }
    fread(header, sizeof(struct FileHeader), 1, f);
    final_header->text_size += header->text_size;
    final_header->data_size += header->data_size;
    return f;
}

int link(const char *out_path, char *object_files[], int file_count, const char *entry_symbol) {
    SourceFile source_files[file_count];

    SymbolTable global_symbols;
    st_init(&global_symbols);

    struct FileHeader final_header;
    final_header.text_size = 0;
    final_header.data_size = 0;

    /*
    For each file:
       - load its text segment
       - load its data segment
       - load its relocation table
       - load its symbol table and add global defined symbols to global symbol table
    */
    for (int file_index = 0; file_index < file_count; file_index++) {
        uint32_t text_offset, data_offset;
        if (file_index == 0) {
            text_offset = 0;
            data_offset = 0;
        }
        else {
            text_offset = final_header.text_size;
            data_offset = final_header.data_size;
        }
        // Open file
        struct FileHeader header;
        FILE *f = open_object_file(object_files[file_index], &final_header, &header);
        if (f == NULL) {
            for (int i = 0; i < file_index; i++) {
                file_destroy(&source_files[i]);
            }
            return 0;
        }

        // Load file
        SourceFile file;
        file_init(&file, text_offset, data_offset, header.text_size, header.data_size);
        load_file(f, &file, &header, &global_symbols);
        source_files[file_index] = file;
        fclose(f);
    }

    // If entry is __start, link __start.o
    SourceFile start;
    int link_start = 0;
    if (entry_symbol != NULL && strcmp(entry_symbol, "__start") == 0) {
        link_start = 1;
        struct FileHeader header;
        const uint32_t text_offset = final_header.text_size;
        const uint32_t data_offset = final_header.data_size;
        FILE *f = open_object_file("__start.o", &final_header, &header);
        if (f == NULL) goto _link_failed;

        file_init(&start, text_offset, data_offset, header.text_size, header.data_size);
        load_file(f, &start, &header, &global_symbols);
        fclose(f);
    }

    /*
    In each relocation table,
    resolve each relocation
    */
    for (int file_index = 0; file_index < file_count; file_index++) {
        file_relocation(&source_files[file_index], &global_symbols);
    }
    if (link_start) {
        file_relocation(&start, &global_symbols);
    }

    // Determine entry
    if (entry_symbol == NULL) {
        final_header.entry = TEXT_START;
    } else {
        Symbol* entry = st_get_symbol_safe(&global_symbols, entry_symbol);
        if (entry == NULL) {
            error_context("Did you add .globl to the entry symbol?");
            return 0;
        }
        final_header.entry = entry->offset;
    }

    /*
    Build file by combining text and data segments
    */
    FILE *out = fopen(out_path, "wb");
    if (out == NULL) {
        raise_error(FILE_IO, out_path, __FILE__);
        goto _link_failed;
    }
    fwrite(&final_header, sizeof(struct FileHeader), 1, out);
    for (int file_index = 0; file_index < file_count; file_index++) {
        // Write text segment
        fwrite(source_files[file_index].text, sizeof(uint32_t), source_files[file_index].text_size/4, out);
    }
    if (link_start) {
        fwrite(start.text, sizeof(uint32_t), start.text_size/4, out);
    }
    for (int file_index = 0; file_index < file_count; file_index++) {
        // Write data segment
        fwrite(source_files[file_index].data, sizeof(uint8_t), source_files[file_index].data_size, out);
    }
    if (link_start) {
        fwrite(start.data, sizeof(uint8_t), start.data_size, out);
    }
    fclose(out);

    for (int file_index = 0; file_index < file_count; file_index++) {
        file_destroy(&source_files[file_index]);
    }
    if (link_start) file_destroy(&start);

    return 1;

    _link_failed:
    for (int file_index = 0; file_index < file_count; file_index++) {
        file_destroy(&source_files[file_index]);
    }
    return 0;
}
