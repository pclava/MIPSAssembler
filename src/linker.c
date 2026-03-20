#include "linker.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include "mman.h"
#else
#include <sys/mman.h>
#endif

static inline const char *get_string(const char *strtab, const uint32_t index) {
    return &strtab[index];
}

// Returns final memory address of a symbol with the formula: section base + object file offset + symbol offset
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

void file_destroy(const SourceFile *file) {
    if (file == NULL) return;
    st_destroy(file->symbol_table);
    free(file->symbol_table);
    if (munmap(file->file.file, file->file.size) == -1) {
        raise_error(MEM, NULL, __FILE__);
    }
}

int relocate(const SourceFile *source, const struct mof_relocation relocation, uint32_t final_address) {
    if (relocation.segment == UNDEF) return 0;
    const uint32_t instr_addr = TEXT_START + source->text_offset + relocation.offset;   // Address of instruction needed relocation (ignored if in data segment)
    const uint32_t instr_index = relocation.offset / 4; // index in local text segment
    struct mof_file file = source->file;
    switch (relocation.type) {
        case R_32:
            if (relocation.segment != DATA) {
                fprintf(stderr, "Error linking %s: attempted R_32 relocation outside data segment\n", source->name);
                return 0;
            }
            // Replace bytes little-endian
            file.data[relocation.offset] = final_address & 0xff;
            file.data[relocation.offset + 1] = final_address >> 8 & 0xff;
            file.data[relocation.offset + 2] = final_address >> 16 & 0xff;
            file.data[relocation.offset + 3] = final_address >> 24 & 0xff;
            break;
        case R_26:
            if (relocation.segment != TEXT) {
                fprintf(stderr, "Error linking %s: attempted R_26 relocation outside text segment\n", source->name);
                return 0;
            }
            // Check range (compare MSBs of instruction's real address and final address)
            if ((instr_addr & 0xF0000000) != (final_address & 0xF0000000)) {
                fprintf(stderr, "Error linking %s: jump target out of range\n", source->name);
                return 0;
            }
            // Zero out lower 26 bits and insert address
            file.text[instr_index] &= 0xfc000000;
            file.text[instr_index] |= (final_address & 0x0fffffff) >> 2;
            break;
        case R_PC16:
            if (relocation.segment != TEXT) {
                fprintf(stderr, "Error linking %s: attempted R_PC16 relocation outside text segment\n", source->name);
                return 0;
            }
            // Check range (within 2^15 instructions)
            const int32_t dist = ((int32_t) final_address - ((int32_t) instr_addr + 4))/4;
            if (dist < INT16_MIN || dist > INT16_MAX) {
                fprintf(stderr, "Error linking %s: branch target out of range\n", source->name);
                return 0;
            }
            file.text[instr_index] &= 0xffff0000;
            file.text[instr_index] |= (uint16_t) dist;
            break;
        case R_HI16:
            if (relocation.segment != TEXT) {
                fprintf(stderr, "Error linking %s: attempted R_HI16 relocation outside text segment\n", source->name);
                return 0;
            }
            file.text[instr_index] &= 0xffff0000;
            file.text[instr_index] |= final_address >> 16;
            break;
        case R_LO16:
            if (relocation.segment != TEXT) {
                fprintf(stderr, "Error linking %s: attempted R_LO16 relocation outside text segment\n", source->name);
                return 0;
            }
            file.text[instr_index] &= 0xffff0000;
            file.text[instr_index] |= final_address & 0x0000ffff;
            break;
        default:
            fprintf(stderr, "Error linking %s: unrecognized relocation directive\n", source->name);
            return 0;
    }
    return 1;
}

int file_relocation(const SourceFile *source, const SymbolTable *global_symbols) {
    const struct mof_relocation *relocation_table = source->file.relocs;
    const char *strtab = source->file.strings;
    const uint32_t text_offset = source->text_offset;
    const uint32_t data_offset = source->data_offset;
    const uint32_t relocation_table_size = source->file.hdr.rels / MOF_RELOCSIZE;
    for (uint32_t i = 0; i < relocation_table_size; i++) {
        const struct mof_relocation relocation = relocation_table[i];

        // Get dependency
        const char *dependency_name = get_string(strtab, relocation.index);
        // const Symbol *dependency = st_get_symbol_by_index(source->symbol_table, strtab, dependency_name);
        const Symbol *dependency = st_get_symbol(source->symbol_table, dependency_name);
        if (dependency == NULL) {
            raise_error(TOKEN_ERR, dependency_name, source->name);
            return 0;
        }

        // Get dependency's final address
        uint32_t final_address = 0;
        switch (dependency->segment) {
            case TEXT:
                final_address = get_final_address(*dependency, text_offset);
                break;
            case DATA:
                final_address = get_final_address(*dependency, data_offset);
                break;
            case UNDEF:
                // Check global symbol table and update dependency
                if (dependency->binding != GLOBAL) {
                    fprintf(stderr, "Error linking %s: symbol undefined\n", source->name);
                    return 0;
                }
                dependency = st_get_symbol(global_symbols, dependency_name);
                if (dependency == NULL) {
                    raise_error(TOKEN_ERR, dependency_name, source->name);
                    if (strcmp(dependency_name, "main") == 0) {
                        error_context("Could not find symbol 'main'. Have you exported it with .globl?");
                    }
                    return 0;
                }
                final_address = dependency->offset; // recall that the GST saves the final address in the offset field
                break;
            default:
                return 0;
        }

        // Resolve relocation
        if (relocate(source, relocation, final_address) == 0) {
            fprintf(stderr, "Error linking %s: relocation failed\n", source->name);
            return 0;
        }
    }

    return 1;
}

void load_symbols(SourceFile *file, SymbolTable *global_symbols) {
    // Read symbol table
    const struct mof_symbol *symbol_table = file->file.syms;
    const char *strtab = file->file.strings;
    const uint32_t symbol_table_size = file->file.hdr.syms / MOF_SYMSIZE;
    for (uint32_t i = 0; i < symbol_table_size; i++) {
        const Symbol symbol = symbol_table[i];

        // Add to local symbol table
        // strtab_debug2(strtab, 204);
        const char *string = get_string(strtab, symbol.index);
        st_add_struct(file->symbol_table, symbol, string);

        // Check if defining global symbol
        if (symbol.binding == GLOBAL && symbol.segment != UNDEF) {
            // Get final offset
            uint32_t final_offset = 0;
            switch (symbol.segment) {
                case TEXT:
                    final_offset = get_final_address(symbol, file->text_offset);
                    break;
                case DATA:
                    final_offset = get_final_address(symbol, file->data_offset);
                    break;
                default: ;
            }

            // Add to global symbols
            st_add_symbol(global_symbols, get_string(strtab, symbol.index), final_offset, symbol.segment, GLOBAL);
        }
    }
}

// Initializes SourceFile structure. Assumes internal mof_file already read
int file_init(SourceFile *file, uint32_t text_offset, uint32_t data_offset) {
    file->text_offset = text_offset;
    file->data_offset = data_offset;

    SymbolTable *symbol_table = malloc(sizeof(SymbolTable));
    if (symbol_table == NULL) {
        raise_error(MEM, NULL, __FILE__);
        return 0;
    }
    try(st_init(symbol_table), 0);
    file->symbol_table = symbol_table;

    return 1;
}

// Opens object file, initializes mof_file
FILE * open_object_file(const char *path, struct mof_header *final_header, struct mof_file *file) {
    // Open file
    FILE *f = fopen(path, "rb+");
    if (f == NULL) {
        raise_error(FILE_IO, path, __FILE__);
        return NULL;
    }

    // Read header
    mof_read_header(f, &file->hdr);
    if (!mof_is_valid(&file->hdr)) {
        raise_error(BAD_FILE, path, __FILE__);
        fclose(f);
        return NULL;
    }

    // Read binary
    if (fseek(f, 0, SEEK_END) != 0) {
        raise_error(FILE_IO, path, __FILE__);
        fclose(f);
        return NULL;
    }
    const long size = ftell(f); // get file size
    rewind(f);

    void *map = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fileno(f), 0);
    if (map == MAP_FAILED) {
        raise_error(FILE_IO, path, __FILE__);
        fclose(f);
        return NULL;
    }
    file->file = map;
    file->size = (uint32_t) size;
    file->text = mof_text(file->file);
    file->data = mof_data(file->file, &file->hdr);
    file->relocs = mof_relocs(file->file, &file->hdr);
    file->syms = mof_symbols(file->file, &file->hdr);
    file->strings = mof_strtab(file->file, &file->hdr);

    // printf("%lu\n", size - MOF_STROFF(&file->hdr));
    // strtab_debug2(file->strings, size - MOF_STROFF(&file->hdr));

    // Update final header
    final_header->text += file->hdr.text;
    final_header->data += file->hdr.data;
    return f;
}

int link(char *object_files[], int file_count, const struct linker_settings options) {

    SourceFile source_files[file_count];

    SymbolTable global_symbols;
    st_init(&global_symbols);

    struct mof_header final_header;
    memset(&final_header, '\0', sizeof(struct mof_header));

    // Load files and prepare for relocation
    for (int file_index = 0; file_index < file_count; file_index++) {
        uint32_t text_offset, data_offset;
        if (file_index == 0) {
            text_offset = 0;
            data_offset = 0;
        }
        else {
            text_offset = final_header.text;
            data_offset = final_header.data;
        }

        // Open file
        SourceFile file;
        file.name = object_files[file_index];
        FILE *f = open_object_file(object_files[file_index], &final_header, &file.file);
        if (f == NULL) {
            for (int i = 0; i < file_index; i++) {
                file_destroy(&source_files[i]);
            }
            return 0;
        }

        // Initialize source file
        file_init(&file, text_offset, data_offset);

        // Load file (populate symbol table)
        load_symbols(&file, &global_symbols);

        source_files[file_index] = file;
        fclose(f);
    }

    // If entry is __start, link __start.o at the end
    SourceFile start;
    start.name = "__start.o";
    if (options.link_start) {
        const uint32_t text_offset = final_header.text;
        const uint32_t data_offset = final_header.data;
        FILE *f = open_object_file("__start.o", &final_header, &start.file);
        if (f == NULL) goto _link_failed;
        file_init(&start, text_offset, data_offset);
        load_symbols(&start, &global_symbols);
        fclose(f);
    }

    // Determine entry
    if (options.entry == NULL) {
        final_header.entry = TEXT_START;
    }
    else {
        Symbol* entry = st_get_symbol_safe(&global_symbols, options.entry);
        if (entry == NULL) {
            error_context("Did you add .globl to the entry symbol?");
            return 0;
        }
        final_header.entry = entry->offset;
    }

    // Create a global symbol __ENTRY pointing to program entry
    if (st_add_symbol(&global_symbols, "__ENTRY", final_header.entry, TEXT, GLOBAL) ==0) goto _link_failed;

    // At this point, every file has been read and we know every final address

    // Go through each file and resolve every relocation
    for (int file_index = 0; file_index < file_count; file_index++) {
        if (file_relocation(&source_files[file_index], &global_symbols) == 0) goto _link_failed;
    }
    if (options.link_start) {
        file_relocation(&start, &global_symbols);
    }

    /*
    Build file by combining text and data segments
    */
    FILE *out = fopen(options.out_path, "wb");
    if (out == NULL) {
        raise_error(FILE_IO, options.out_path, __FILE__);
        goto _link_failed;
    }

    mof_write_header(out, &final_header);

    // Write text segment(s)
    for (int file_index = 0; file_index < file_count; file_index++) {
        SourceFile file = source_files[file_index];
        fwrite(file.file.text, sizeof(uint32_t), file.file.hdr.text/4, out);
    }
    if (options.link_start) {
        fwrite(start.file.text, sizeof(uint32_t), start.file.hdr.text/4, out);
    }
    // Write data segment(s)
    for (int file_index = 0; file_index < file_count; file_index++) {
        SourceFile file = source_files[file_index];
        fwrite(file.file.data, sizeof(uint8_t), file.file.hdr.data, out);
    }
    if (options.link_start) {
        fwrite(start.file.data, sizeof(uint8_t), start.file.hdr.data, out);
    }
    fclose(out);

    for (int file_index = 0; file_index < file_count; file_index++) {
        file_destroy(&source_files[file_index]);
    }
    if (options.link_start) file_destroy(&start);

    if (options.dump_symbols != NULL) {
        st_dump(&global_symbols, options.dump_symbols);
    }
    st_destroy(&global_symbols);
    return 1;

    _link_failed:
    for (int file_index = 0; file_index < file_count; file_index++) {
        file_destroy(&source_files[file_index]);
    }
    return 0;
}

/*
 Main function for the linker
 Usage:
 MIPSLinker <options> <files>

 Expects as input at least one object file generated by the assembler
 Outputs an executable

 Options:
 -c             : deletes sources object files after linking (be careful!)
 -d <path>      : dumps the global symbol table to the given path for debugging purposes
 -o <path>      : writes the executable to the given path. if not provided, writes to "a.out"
 -s <symbol>    : begin program execution at the given symbol. if not provided, begins at first instruction
 -ls            : links __start.o and sets program entry to __start. overrides -s if put after
 */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "error in %s: invalid arguments\n", __FILE__);
        return 1;
    }

    // Settings for linker
    char **in_files = malloc((argc-1) * sizeof(char *));
    if (in_files == NULL) {
        raise_error(MEM, NULL, __FILE__);
        return 1;
    }
    char default_out[] = "a.out";
    struct linker_settings options = {0, 0, NULL, default_out, NULL};
    char start_sym[] = "__start";

    // Get input files
    int file_count = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'c':
                    options.clean = 1;
                    break;
                case 'd':
                    i++;
                    options.dump_symbols = argv[i];
                    break;
                case 'o':
                    i++;
                    options.out_path = argv[i];
                    break;
                case 's':
                    i++;
                    options.entry = argv[i];
                    break;
                case 'l':
                    if (argv[i][2] == 's') {
                        options.link_start = 1;
                        options.entry = start_sym;
                        break;
                    }
                    fprintf(stderr, "error in %s: skipping unrecognized option %s\n", __FILE__, argv[i]); // to avoid gcc's "this statement may fall through" warning
                    break;
                default:
                    fprintf(stderr, "error in %s: skipping unrecognized option %s\n", __FILE__, argv[i]);
            }
            continue;
        }
        in_files[file_count++] = argv[i];
    }

    // Link files
    if (link(in_files, file_count, options) == 0) {
        fprintf(stderr, "Error in %s: could not link files\n", __FILE__);
        free(in_files);
        return 2;
    }

    // Clean
    if (options.clean) {
        for (int i = 0; i < file_count; i++) {
            remove(in_files[i]);
        }
    }

    free(in_files);
    return 0;
}