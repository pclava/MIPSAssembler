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

// reads an immediate of the form %OPERATOR(SYMBOL)
// where OPERATOR can be hi or lo
// fills given Imm pointer and returns pointer to end of expression
const char * read_operator(Immediate * imm, const char *str, SymbolTable * symbol_table) {
    imm->modifier = 0;
    imm->type = SYMBOL;

    char operator[32];
    memset(operator, 0, sizeof(operator));
    // Copy until parenthesis or size exceeded
    int i = 0;
    int j = 0;
    while (i < 31 && str[j] != '(' && str[j] != '\0') {
        operator[i++] = str[j++];
    }
    if (str[j] == '\0' || i == 31) {
        raise_error(ARG_INV, str, __FILE__);
        return NULL;
    }
    j++; // skip parenthesis

    if (strcmp(operator, "%hi") == 0) imm->modifier = 1;
    else if (strcmp(operator, "%lo") == 0) imm->modifier = 2;
    else {
        raise_error(ARGS_INV, str, __FILE__);
        return NULL;
    }

    // Read symbol until close parenthesis
    String *string = malloc(sizeof(Symbol));
    if (string_init(string) == 0) return NULL;

    i = 0;
    while (i < 31 && str[j] != ')' && str[j] != '\0') {
        string_insert(string, i++, str[j++]);
    }
    if (*str == '\0' || i == 31) {
        raise_error(ARG_INV, str, __FILE__);
        return NULL;
    }

    // Get symbol
    if (st_exists(symbol_table, string->str) == SYMBOL_TABLE_SIZE) {
        if (st_add_symbol(symbol_table, string->str, 0, UNDEF, LOCAL) == 0) return 0;
    }
    Symbol *s = st_get_symbol(symbol_table, string->str);

    imm->symbol = s;
    string_destroy(string);
    return &str[++j];
}

// Parses a string into an Immediate struct
Immediate parse_imm(const char * str, SymbolTable *symbol_table) {
    Immediate imm;
    imm.modifier = 0;
    imm.type = NONE;
    imm.intValue = 0;
    imm.rgstr = 0;

    const char *endptr;

    if (str[0] == '%') {
        endptr = read_operator(&imm, str, symbol_table);
        if (endptr == NULL) {
            imm.modifier = 255;
            return imm;
        }
    }

    // CHARACTER
    else if (str[0] == '"') {
        if (str[1] == '\\') {
            char c;
            const size_t x = read_escape_sequence(&str[1], &c);
            if (x == 0) {
                raise_error(ARG_INV, str, __FILE__);
                imm.modifier = 255;
                return imm;
            }
            endptr = str+x+1;
            if (*endptr != '"') {
                raise_error(ARG_INV, str, __FILE__);
                imm.modifier = 255;
                return imm;
            }
            endptr++;
            imm.intValue = (int32_t) c;
            imm.type = NUM;
        }
        else if (str[2] == '"') {
            imm.intValue = (int32_t) str[1];
            imm.type = NUM;
            endptr = &str[3];
        }
        else {
            raise_error(ARG_INV, str, NULL);
            imm.modifier = 255;
            return imm;
        }
    }

    // NUMBER
    else if (isdigit(str[0]) || str[0] == '-') {
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

        char *ep;
        const long n = strtol(str, &ep, base);
        endptr = ep;

        imm.intValue = (int32_t) n;
    }

    // SYMBOL
    else {
        // Finds the symbol or adds it if not found
        if (st_exists(symbol_table, str) == SYMBOL_TABLE_SIZE) {
            // Doesn't exist yet, add as local undefined. may be made global later
            st_add_symbol(symbol_table, str, 0, UNDEF, LOCAL);
        }
        Symbol *s = st_get_symbol(symbol_table, str);

        imm.symbol = s;
        imm.type = SYMBOL;
        return imm;
    }

    if (*endptr == '\0') return imm;

    if (*endptr != '(') {
        raise_error(ARG_INV, str, __FILE__);
        imm.modifier = 255;
        return imm;
    }
    endptr++;

    // REGISTER OFFSET
    // save register in imm.rgstr
    // immediate is either a number or a symbol
    // we know which by the modifier (1 or 2 for symbol, 0 for number)
    if (imm.type == SYMBOL && imm.modifier == 0) {
        raise_error(ARG_INV, str, __FILE__);
        imm.modifier = 255;
        return imm;
    }
    if (imm.type == NUM && imm.modifier != 0) {
        raise_error(ARG_INV, str, __FILE__);
        imm.modifier = 255;
        return imm;
    }
    imm.type = REG_OFFSET;
    // read until close parenthesis
    char rgstr[32];
    memset(rgstr, '\0', sizeof(rgstr));
    int i = 0;
    while (endptr[i] != ')') {
        if (i > 31 || *endptr == '\0') {
            raise_error(ARG_INV, str, __FILE__);
            imm.modifier = 255;
            return imm;
        }
        rgstr[i] = endptr[i];
        i++;
    }
    imm.rgstr = get_register(rgstr);
    if (imm.rgstr == 255) {
        raise_error(ARG_INV, str, __FILE__);
        imm.modifier = 255;
        return imm;
    }

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
            fprintf(stderr, "-> symbols can only contain alphanumeric including ._$ and cannot begin with a number\n");
            break;
        case ARG_INV:
            fprintf(stderr, "-> invalid argument \"%s\"\n", object);
            break;
        case ARGS_INV:
            fprintf(stderr, "-> invalid arguments to instruction or directive\n");
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
// Puts length of string in len, returns pointer to dst
char * read_string(char *dst, size_t *dst_size, char *token, int *len) {
    if (*dst_size <= 1 || token[0] != '\"') {
        raise_error(NOERR, NULL, __FILE__);
        return NULL;
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
                return NULL;
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
                return NULL;
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
            return NULL;
        }
        dst = new;
    }

    dst[j] = '\0';

    // Check if characters remain in buffer
    if (token[++i] != '\0') {
        raise_error(ARG_INV, dst, __FILE__);
        free(dst);
        return NULL;
    }

    *len = (int) j-1;
    return dst; // j counts number of characters + initial quote
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

// Returns whether c is a valid symbol character
int is_symbol(char c) {
    return isalnum(c) || c == '_' || c == '$' || c == '.';
}