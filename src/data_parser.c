#include "data_parser.h"
#include "utils.h"
#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Data parser

These functions handle everything related to the data segment, namely the DataList structure
and the parsing of tokens into the correct data type.
*/

int word(Data *data, const char * str, SymbolTable *symbol_table) {
    data->size = 4;
    data->type = WORD;
    data->isSymbol = 0;

    Immediate imm = parse_imm(str, symbol_table, 0);
    if (imm.modifier == 255) {
        return 0;
    }

    if (imm.type == SYMBOL) {
        data->isSymbol = 1;
        data->value.symbol = imm.symbol;
        return 1;
    }

    if (imm.intValue < INT_MIN || imm.intValue > INT_MAX) {
        raise_error(ARG_INV, str, __FILE__);
        return 0;
    }

    data->value.word = imm.intValue;
    return 1;
}

int half(Data * data, const char * str, SymbolTable *symbol_table) {
    data->size = 2;
    data->isSymbol = 0;
    data->type = HALF;

    Immediate imm = parse_imm(str, symbol_table, 0);
    if (imm.modifier == 255) {
        return 0;
    }

    if (imm.type == SYMBOL) {
        data->isSymbol = 1;
        data->value.symbol = imm.symbol;
        return 1;
    }

    if (imm.intValue < SHRT_MIN || imm.intValue > USHRT_MAX) {
        raise_error(ARG_INV, str, __FILE__);
        return 0;
    }

    data->value.half = (int16_t) imm.intValue;
    return 1;
}

int byte(Data * data, const char * str, SymbolTable *symbol_table) {
    data->size = 1;
    data->isSymbol = 0;
    data->type = BYTE;

    Immediate imm = parse_imm(str, symbol_table, 0);
    if (imm.modifier == 255) {
        return 0;
    }

    if (imm.type == SYMBOL) {
        data->isSymbol = 1;
        data->value.symbol = imm.symbol;
        return 1;
    }

    if (imm.intValue < CHAR_MIN || imm.intValue > UCHAR_MAX) {
        raise_error(ARG_INV, str, __FILE__);
        return 0;
    }

    data->value.byte = (int8_t) imm.intValue;
    return 1;
}

// Note that strings have an initial " to differentiate from other arguments
// The final " will have been stripped during parsing
int string(Data * data, const char * str, SymbolTable *symbol_table) {
    (void)symbol_table; // to suppress unused parameter warning
    // assumes caller has filled in the size
    if (str[0] != '\"') {
        raise_error(ARG_INV, str, __FILE__);
        return 0;
    }
    // if no closing quote
    if (str[data->size+1] != '\"') {
        raise_error(ARG_INV, str, __FILE__);
        return 0;
    }
    data->type = STRING;
    data->isSymbol = 0;
    data->value.string = strdup(str+1);
    return 1;
}

int string_nt(Data * data, const char *str, SymbolTable *symbol_table) {
    (void)symbol_table; // to suppress unused parameter warning
    // assumes caller has filled in the size
    if (str[0] != '\"') {
        raise_error(ARG_INV, str, __FILE__);
        return 0;
    }
    // if no closing quote
    if (strrchr(str, '"') == str) {
        raise_error(ARG_INV, str, __FILE__);
        return 0;
    }
    data->type = STRING_NT;
    data->isSymbol = 0;
    data->value.string = strdup(str+1);
    return 1;
}

int (*PROCESS_DATA[5])(Data *, const char *, SymbolTable *) = {
    &word, &half, &byte, &string, &string_nt
};

// Parses a string into the Data structure depending on its type
int process_data(Data * data, const enum DataType data_type, const char * str, SymbolTable *symbol_table) {
    return PROCESS_DATA[data_type](data, str, symbol_table);
}

// Initialize a DataList with data addresses beginning at 'entry'
int dl_init(DataList * data_list, uint32_t entry) {
    data_list->len = 0;
    data_list->cap = 64;
    data_list->list = malloc(data_list->cap * sizeof(Data));
    if (data_list->list == NULL) {
        raise_error(MEM, NULL, __FILE__);
        return 0;
    }
    data_list->data_offset = entry;
    return 1;
}

// Adds Data struct to DataList, increments data_addr, returns success
int add_data(DataList *data_list, const Data data) {

    if (data_list->len >= data_list->cap) {
        data_list->cap *= 2;
        Data *new = realloc(data_list->list, data_list->cap*sizeof(Data));
        if (new == NULL) {
            raise_error(MEM, NULL, __FILE__);
            return 0;
        }
        data_list->list = new;
    }

    data_list->list[data_list->len] = data;
    data_list->len++;

    data_list->data_offset += data.size;
    return 1;
}

// Free resources
void dl_destroy(const DataList *data_list) {
    for (size_t i = 0; i < data_list->len; i++) {
        if (data_list->list[i].type == STRING || data_list->list[i].type == STRING_NT) {
            if (data_list->list[i].value.string != NULL) {
                free(data_list->list[i].value.string);
            }
        }
    }
    free(data_list->list);
}

void dl_debug(const DataList *data_list) {
    for (size_t i = 0; i < data_list->len; i++) {
        data_debug(data_list->list[i]);
    }
    printf("data size: %d\n", data_list->data_offset);
}

void data_debug(const Data data) {
    switch (data.type) {
        case WORD:
            if (data.isSymbol) printf("WORD value=symbol number %d, bytes=%d\n", data.value.symbol->index, data.size);
            else printf("WORD value=%d bytes=%d\n", data.value.word, data.size);
            break;
        case HALF:
            printf("HALF value=%d bytes=%d\n", data.value.half, data.size);
            break;
        case BYTE:
            printf("BYTE value=%d bytes=%d\n", data.value.byte, data.size);
            break;
        case STRING:
            printf("STRING\n");
            break;
        case STRING_NT:
            printf("STRING_NT value=\"%s\" bytes=%d\n", data.value.string, data.size);
            break;
        case SPACE:
            printf("SPACE bytes=%d\n", data.size);
            break;
        default:
            printf("UNKNOWN DATA TYPE\n");
    }
}

// Updates the DataType variable at the given pointer based on the input string
int read_directive(const char *directive, enum DataType *type) {
    if (directive[0] != '.') {
        raise_error(ARG_INV, directive, __FILE__);
        return 0;
    }
    directive++;
    if (strcmp(directive, "align") == 0) {
        *type = ALIGN;
    } else if (strcmp(directive, "ascii") == 0) {
        *type = STRING;
    } else if (strcmp(directive, "asciiz") == 0) {
        *type = STRING_NT;
    } else if (strcmp(directive, "byte") == 0) {
        *type = BYTE;
    } else if (strcmp(directive, "word") == 0) {
        *type = WORD;
    } else if (strcmp(directive, "space") == 0) {
        *type = SPACE;
    } else if (strcmp(directive, "half") == 0) {
        *type = HALF;
    } else {
        raise_error(TOKEN_ERR, directive, __FILE__);
        return 0;
    }

    return 1;
}

/* Returns the number of bytes needed to increment data_addr such that
it is on a given boundary (where for type: 0 = byte; 1 = half; 2 = word; 3 = double) */
uint32_t data_align(const int type, const DataList *data_list) {
    int x;
    switch (type) {
        case 0:
            x = 1;
            break;
        case 1:
            x = 2;
            break;
        case 2:
            x = 4;
            break;
        case 3:
            x = 8;
            break;
        default:
            return -1;
    }

    uint32_t bytes = 0;
    while ((data_list->data_offset + bytes) % x != 0) bytes++;
    return bytes;
}

// Adds necessary padding bytes such that .word and .half directives begin on the correct boundaries
int data_pad(const Data data, DataList *data_list) {
    int n = 0;
    if (data.type == WORD) {
        n = 2;
    } else if (data.type == HALF) {
        n = 1;
    }
    const uint32_t bytes = data_align(n, data_list);
    return add_padding(data.line, bytes, data_list);
}

// Adds a Data structure of type SPACE
int add_padding(const Line *line, const uint32_t bytes, DataList * data_list) {
    try(bytes, 1);
    Data padding;
    padding.type = SPACE;
    padding.size = bytes;
    padding.isSymbol = 0;
    padding.value.byte = 0;
    padding.line = line;
    return add_data(data_list, padding);
}

// Adds padding bytes to the DataList such that it is aligned on a given boundary (.align directive)
int add_aligned(const Line *line, const char *token, DataList * data_list) {
    char *endptr;
    const long n = strtol(token, &endptr, 10);
    if (*endptr != '\0') {
        raise_error(ARG_INV, token, __FILE__);
        return 0;
    }

    const uint32_t bytes = data_align((int) n, data_list);
    if (bytes == (uint32_t) -1) {
        raise_error(ARG_INV, token, __FILE__);
        return 0;
    }

    // Create padding
    const int x = add_padding(line, bytes, data_list);
    if (x == 0) {
        raise_error(ARG_INV, token, __FILE__);
        return 0;
    }
    return x;
}

// Adds any number of padding bytes (.space directive)
int add_space(const Line *line, const char *token, DataList * data_list) {
    char *endptr;
    const long n = strtol(token, &endptr, 10);
    if (*endptr != '\0' || n <= 0) {
        raise_error(ARG_INV, token, __FILE__);
        return 0;
    }

    // Create padding
    const int x = add_padding(line, n, data_list);
    if (x == 0) {
        raise_error(ARG_INV, token, __FILE__);
        return 0;
    }
    return x;
}