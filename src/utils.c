#include "utils.h"

#include "symbol_table.h"
#include "reloc_table.h"

#include <stdlib.h>
#include <ctype.h>
#include <stddef.h>
#include <string.h>

const char *REGISTERS[REGISTER_COUNT] = {
    "$zero", "$at", "$v0", "$v1", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2", "$t3", "$t4", "$t5",
    "$t6", "$t7", "$s0", "$s1", "$s2", "$s3", "$s4", "$s5", "$s6", "$s7", "$t8", "$t9", "$k0", "$k1", "$gp",
    "$sp", "$fp", "$ra"
};


// Parses a string into an Immediate struct
// First checks if the string is a character, otherwise if it's a number, and finally assumes it's a symbol
Immediate parse_imm(const char * str) {
    Immediate imm;
    imm.modifier = 0; // only used in special cases
    imm.type = NONE;
    imm.intValue = 0;
    const size_t len = strlen(str);

    // CHARACTER
    if (str[0] == '"') {
        if (len == 3) {
            imm.intValue = (int32_t) str[1];
            imm.type = NUM;
            return imm;
        }
        if (len > 3 && str[1] == '\\') {
            char c;
            if (read_escape_sequence(&str[1], &c) == 0) {
                raise_error(ARG_INV, str, __FILE__);
                imm.modifier = 255;
                return imm;
            }

            imm.intValue = (int32_t) c;
            imm.type = NUM;
            return imm;
        }

        raise_error(ARG_INV, str, NULL);
        imm.modifier = 255;
        return imm;
    }

    // NUMBER
    if (isnumber(str[0]) || str[0] == '-') {
        imm.type = NUM;
        int base = 10; // assume 10
        if (str[0] == '0') {
            if (isalpha(str[1])) {
                switch (str[1]) {
                    case 'B':
                    case 'b':
                        base = 2;
                        break;
                    case 'X':
                    case 'x':
                        base = 16;
                        break;
                    default:
                        raise_error(ARG_INV, str, __FILE__);
                        imm.modifier = 255;
                        return imm;
                }
            } else base = 8;
        }

        char *endptr;
        const long n = strtol(str, &endptr, base);
        if (*endptr == '(') {
            // Register-relative address
            // Second pass will catch further errors
            imm.type = REG_OFFSET;
            strcpy(imm.symbol, str);
            return imm;
        }
        if (*endptr != '\0') {
            raise_error(ARG_INV, str, __FILE__);
            imm.modifier = 255;
            return imm;
        }

        imm.intValue = (int32_t) n;
        return imm;
    }

    // SYMBOL
    if (len >= SYMBOL_SIZE) {
        raise_error(ARG_INV, str, __FILE__);
        imm.modifier = 255;
        return imm;
    }

    strcpy(imm.symbol, str);
    imm.type = SYMBOL;
    return imm;
}

int write_byte(FILE *file, const uint8_t byte) {
    if (fwrite(&byte, 1, 1, file) == 0) return 0;
    return 1;
}

int write_word(FILE *file, const uint32_t word) {
    if (fwrite(&word, 4, 1, file) == 0) return 0;
    return 1;
}

int write_half(FILE *file, const uint16_t half) {
    if (fwrite(&half, 2, 1, file) == 0) return 0;
    return 1;
}

int write_string(FILE *file, const char *str, const uint32_t len) {
    for (size_t i = 0; i < len; i++) {
        char c = str[i];
        if (write_byte(file, str[i]) == 0) {
            return 0;
        }
    }
    return 1;
}

uint8_t read_byte(FILE *file) {
    int8_t byte;
    fread(&byte, sizeof(byte), 1, file);
    return byte;
}

uint32_t read_word(FILE *file) {
    uint32_t word;
    fread(&word, sizeof(word), 1, file);
    return word;
}

ErrorHandler ERROR_HANDLER = {
    NULL,
    NOERR,
    NULL,
    NULL
};

// Updates the parameters of ERROR_HANDLER, calls error()
void raise_error(const errcode errcode, const char * errobj, const char * file) {
    ERROR_HANDLER.err_code = errcode;
    ERROR_HANDLER.file = file;
    ERROR_HANDLER.err_obj = errobj;
    error();
}

// Prints an error message based on ERROR_HANDLER
// Calls either general_error() or assembler_error()
void error(void) {
    // No error code given
    if (ERROR_HANDLER.err_code == 0) {
        if (ERROR_HANDLER.file == NULL) {
            fprintf(stderr, "An error occured\n");
            return;
        }
        fprintf(stderr, "In %s: an error occured\n", ERROR_HANDLER.file);
    }

    // General error
    if (ERROR_HANDLER.err_code >= 1 && ERROR_HANDLER.err_code <= 2) {
        if (ERROR_HANDLER.file == NULL) {
            fprintf(stderr, "An error occured\n");
            return;
        }
        general_error(ERROR_HANDLER.err_code, ERROR_HANDLER.file, ERROR_HANDLER.err_obj);
    }

    // Assembler error
    if (ERROR_HANDLER.err_code > 2) {
        if (ERROR_HANDLER.line == NULL) {
            fprintf(stderr, "An error occured\n");
            return;
        }
        assembler_error(ERROR_HANDLER.err_code, ERROR_HANDLER.line, ERROR_HANDLER.err_obj);
        return;
    }
}

// Provides more context to the error
void error_context(const char * str) {
    fprintf(stderr, "-> (%s)\n", str);
}

void general_error(const errcode code, const char *file, const char * object) {
    fprintf(stderr, "Error in %s:\n  ", file);
    switch (code) {
        case FILE_IO:
            fprintf(stderr, "-> could not access file \"%s\"\n", object);
            break;
        case MEM:
            fprintf(stderr, "-> could not allocate memory\n");
            break;
        case NOERR:
            fprintf(stderr, "-> an error occurred\n");
            break;
        default:
            fprintf(stderr, "-> unrecognized error\n");
    }
}

void assembler_error(const errcode code, const Line *line, const char * object) {
    fprintf(stderr, "Error in %s:%d\n    %s\n    ", line->filename, line->number, line->text);
    switch (code) {
        case TOKEN_ERR:
            fprintf(stderr, "-> unrecognized token \"%s\"\n", object);
            break;
        case SYMBOL_INV:
            fprintf(stderr, "-> invalid symbol definition \"%s\"\n    ", object);
            fprintf(stderr, "-> symbols must be between 1 and %d characters long and must be alphanumeric\n", SYMBOL_SIZE-1);
            break;
        case ARG_INV:
            fprintf(stderr, "-> invalid argument \"%s\"\n", object);
            break;
        case ARGS_INV:
            fprintf(stderr, "-> invalid arguments to instruction or directive\n");
            break;
        case ST_SIZE_ERR:
            fprintf(stderr, "-> too many symbols, could not save symbol \"%s\"\n    -> the assembler supports up to 256 symbols\n", object);
            break;
        case DUPL_DEF:
            fprintf(stderr, "-> token \"%s\" already defined\n", object);
            break;
        case NOERR:
            fprintf(stderr, "-> an error occurred\n");
            break;
        case SIZE_ERR:
            fprintf(stderr, "-> token \"%s\" exceeds expected length\n", object);
            break;
        default:
            fprintf(stderr, "-> unrecognized error\n");
    }
}

// Generates a hash key for the hash tables used by the SymbolTable and InstructionTable structures
// uses djb2 hash (source: https://gist.github.com/MohamedTaha98/ccdf734f13299efb73ff0b12f7ce429f)
unsigned long hash_key(const char *key, const size_t table_size) {
    unsigned long hash = 5381;
    int c;
    while ((c = (unsigned char) *key++))
        hash = (hash << 5) + hash + c;
    return hash % table_size;
}

// Takes as input the pointer to the beginning of an escape sequence, writes the corresponding character to res
// Returns the length of the escape sequence, or 0 on failure
size_t read_escape_sequence(const char *inp, char *res) {
    if (inp[0] != '\\') {
        return 0;
    }
    size_t len = 1;

    // OCTAL
    if (isdigit(inp[1])) {
        char *endptr;
        long c = strtol(inp+1, &endptr, 8);
        if (c < 0 || c > 255) {
            raise_error(ARG_INV, inp, __FILE__);
            return 0;
        }
        *res = (char) c;
        ptrdiff_t diff = endptr - inp;
        len = (size_t) diff;
    }

    // HEX
    else if (inp[1] == 'x' || inp[1] == 'X') {
        char *endptr;
        long c = strtol(inp+2, &endptr, 16);
        if (c < 0 || c > 255) {
            raise_error(ARG_INV, inp, __FILE__);
            return 0;
        }
        *res = (char) c;
        ptrdiff_t diff = endptr - inp;
        len = (size_t) diff;
    }

    // NORMAL ESCAPE SEQUENCE
    else {
        len++;
        char c = inp[1];
        if (c == '\0') {
            raise_error(ARG_INV, inp, __FILE__);
            return 0;
        }

        switch (c) {
            case 'a':
                c = '\a';
                break;
            case 'b':
                c = '\b';
                break;
            case 'f':
                c = '\f';
                break;
            case 'n':
                c = '\n';
                break;
            case 'r':
                c = '\r';
                break;
            case 't':
                c = '\t';
                break;
            case 'v':
                c = '\v';
                break;
            case '\\':
                c = '\\';
                break;
            case '\"':
                c = '\"';
                break;
            case '\'':
                c = '\'';
                break;
            default:
                raise_error(ARG_INV, inp, __FILE__);
                return 0;
        }

        *res = c;
    }

    return len;
}

char * TOKENIZE_START = NULL; // Used by tokenize(): saves the index of the first character of the next token

// Tokenize the string given a single delimeter
// Unlike strtok(), tokenize() does not skip consecutive delimeters, instead stopping at each one.
// To continue tokenizing the same string, pass NULL as the input.
// Returns NULL when there are no more tokens to read.
char * tokenize(char *str, const char delim) {
    // First call
    if (str != NULL) {
        TOKENIZE_START = str;
    }

    // TOKENIZE_START is null or points to '\0' when tokenize() has never been called or when tokenize() reached the end of a string
    if (TOKENIZE_START == NULL || *TOKENIZE_START == '\0') {
        return NULL;
    }

    int i = 0;
    char c;
    while ((c = TOKENIZE_START[i]) != delim) {
        if (c == '\0') {
            char *old = TOKENIZE_START;
            TOKENIZE_START = TOKENIZE_START + i; // end on null terminator
            return old;
        }
        i++;
    }

    TOKENIZE_START[i] = '\0'; // Replace delimeter
    char *old = TOKENIZE_START;
    TOKENIZE_START = TOKENIZE_START + i + 1;
    return old;
}

// Using tokenize(), writes a quotation-mark-wrapped string to dst, with the final quote stripped and escape characters processed
// Expects as input the destination buffer, a pointer to its length, and the first token of the string, generated by tokenize()
// Returns length of string, or -1 on error
int read_string(char *dst, size_t *dst_size, char *token) {
    if (*dst_size <= 1 || token[0] != '\"') {
        raise_error(NOERR, NULL, __FILE__);
        return -1;
    }
    dst[0] = '\"';
    char c = token[1];
    size_t i = 1, j = 1; // index in token, index in argument

    // Loop until closing quote is found
    while (c != '\"') {
        if (c == '\0') {
            token = tokenize(NULL, ' ');
            if (token == NULL) {
                dst[j] = '\0';
                raise_error(ARG_INV, dst, __FILE__);
                free(dst);
                return -1;
            }

            // Add a space because tokenize() drops it
            dst[j] = ' ';
            i = 0;
            j++;
            c = token[i];
            continue;
        }

        // Resize buffer
        if (j >= *dst_size) {
            *dst_size = *dst_size * 2;
            char *new = realloc(dst, *dst_size);
            if (new == NULL) {
                raise_error(MEM, NULL, __FILE__);
                free(dst);
                return -1;
            }
            dst = new;
        }

        // Write to string
        if (token[i] == '\\') {
            char C;
            const size_t offset = read_escape_sequence(&token[i], &C);
            if (offset == 0) return 0;
            dst[j] = C;
            i += (int) offset-1;
        }

        else {
            dst[j] = c;
        }
        i++;
        j++;
        c = token[i];
    }

    // Add null terminator
    if (j >= *dst_size) {
        *dst_size = *dst_size * 2;
        char *new = realloc(dst, *dst_size);
        if (new == NULL) {
            raise_error(MEM, NULL, __FILE__);
            free(dst);
            return -1;
        }
        dst = new;
    }

    dst[j] = '\0';

    // Check if characters remain in buffer
    if (token[++i] != '\0') {
        raise_error(ARG_INV, dst, __FILE__);
        free(dst);
        return -1;
    }

    return (int) j-1; // j counts number of characters + initial quote
}

// Returns the register number associated with a token beginning with '$'. Returns 255 on error.
unsigned char get_register(const char *token) {
    if (token[0] != '$') return 255;
    unsigned char j = 0;
    while (j < REGISTER_COUNT && strcmp(token, REGISTERS[j]) != 0) j++;
    if (j < REGISTER_COUNT) return j;
    char *endptr;
    const long n = strtol(&token[1], &endptr, 10);
    if (*endptr != '\0' || n < 0 || n > 31) return 255;
    return (unsigned char) n;
}

// Expects as input a base-offset address of the form NUMBER(REGISTER). Allows the form (REGISTER) with an implied offset of 0
// Writes the number to the Immediate structure and returns the register number, or -1 on failure.
int read_base_address(const char *str, Immediate *imm) {
    if (str[strlen(str)-1] != ')') { // Should end in ')'
        return -1;
    }

    char address[strlen(str)+1];
    strcpy(address, str);

    // Parse string into immediate and register
    char const *immToken = address;
    const char *rToken = NULL;
    for (size_t i = 0; i < strlen(str); i++) {
        if (address[i] == '('){
            address[i] = '\0';
            rToken = address+i+1;
        }
        if (address[i] == ')') {
            if (rToken == NULL) return -1;
            address[i] = '\0';
        }
    }

    Immediate i;
    i.modifier = 0;
    if (*immToken == '\0') {
        i.type = NUM;
        i.intValue = 0;
    } else {
        i = parse_imm(immToken);
        if (i.modifier == 255) {
            return -1;
        }
        if (i.type != NUM) return -1;
    }

    const int ret = get_register(rToken);
    if (ret == 255) return -1;

    *imm = i;
    return ret;
}

void debug_binary(const char *name) {
    FILE *file = fopen(name, "rb");
    struct FileHeader header;
    fread(&header, sizeof(header), 1, file);
    printf("text size: %d\n", header.text_size);
    printf("data size: %d\n", header.data_size);
    printf("entry: %d\n", header.entry);

    printf("\n");
    for (size_t i = 0; i < header.text_size/4; i++) {
        uint32_t text = read_word(file);
        printf("instruction: 0x%.8x\n", text);
    }

    printf("\n");
    for (size_t i = 0; i < header.data_size; i++) {
        unsigned char c = read_byte(file);
        printf("byte: 0x%.2x (%c)\n", c, c);
    }

    printf("\n");
    uint32_t rlc_size = read_word(file);
    for (size_t i = 0; i < rlc_size; i++) {
        RelocationEntry entry;
        fread(&entry, sizeof(RelocationEntry), 1, file);
        char s[6];
        if (entry.segment == TEXT)
            strcpy(s, ".text");
        else if (entry.segment == DATA)
            strcpy(s, ".data");

        printf("address at %s+%d needs relocation of type %d for symbol %s\n", s, entry.target_offset, entry.reloc_type, entry.dependency);
    }

    printf("\n");
    uint32_t sym_size = read_word(file);
    for (size_t i = 0; i < sym_size; i++) {
        char sym_name[SYMBOL_SIZE];
        uint32_t offset;
        enum Segment segment;
        enum Binding binding;
        fread(&sym_name, SYMBOL_SIZE*sizeof(char), 1, file);
        fread(&offset, sizeof(uint32_t), 1, file);
        fread(&segment, sizeof(enum Segment), 1, file);
        fread(&binding, sizeof(enum Binding), 1, file);
        char s[6];
        if (segment == TEXT) {
            strcpy(s, ".text");
        } else if (segment == DATA) {
            strcpy(s, ".data");
        } else {
            strcpy(s, "UNDEF");
        }
        printf("symbol %s at %s+%d (binding %d)\n", sym_name, s, offset, binding);
    }

    fclose(file);
}