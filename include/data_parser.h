#ifndef MIPS_ASSEMBLER_DATA_PARSER_H
#define MIPS_ASSEMBLER_DATA_PARSER_H
#include <stdint.h>

#include "symbol_table.h"

/* === TYPES === */

enum DataType {
    WORD,      // .word
    HALF,      // .half
    BYTE,      // .byte
    STRING,    // Not null-terminated (.ascii)
    STRING_NT, // Null-terminated string (.asciiz)
    SPACE,     // reserves given number of bytes
    ALIGN      // pseudo-type; becomes a SPACE type
};

typedef struct {
    enum DataType type;
    union {
        int32_t word;
        int16_t half;
        int8_t byte;
        char *string;
        char symbol[SYMBOL_SIZE];
    } value;
    uint32_t size;
    unsigned char isSymbol;
    const Line *line; // corresponding line in the Text list
} Data;

typedef struct {
    size_t len;
    size_t cap;
    uint32_t data_offset;
    Data *list;
} DataList;

/* === DATA PARSING === */

// Parse the given string into the data struct
// Return boolean success
int word(Data *, const char *);
int half(Data *, const char *);
int byte(Data *, const char *);
int string(Data *, const char *);
int string_nt(Data *, const char *);

extern int (*PROCESS_DATA[5])(Data *, const char *);

int process_data(Data *data, enum DataType data_type, const char *str);

/* === DATALIST METHODS === */

int dl_init(DataList * data_list, uint32_t entry);

int add_data(DataList * data_list, Data data);

void dl_destroy(const DataList * data_list);

void dl_debug(const DataList * data_list);

void data_debug(Data data);

/* === DATA PARSING METHODS === */

int read_directive(const char *directive, enum DataType *type);

uint32_t data_align(int type, const DataList *);

int data_pad(Data data,  DataList * data_list);

int add_padding(const Line *line, uint32_t bytes, DataList *data_list);

int add_aligned(const Line *line, const char *token, DataList *data_list);

int add_space(const Line *line, const char *token, DataList *data_list);

#endif //MIPS_ASSEMBLER_DATA_PARSER_H