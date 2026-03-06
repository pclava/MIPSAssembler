#ifndef MIPS_ASSEMBLER_UTILS_H
#define MIPS_ASSEMBLER_UTILS_H
#include <stdint.h>
#include <stdio.h>
#include "text.h"

/* === CONSTANTS === */
#define MAX_5U 31
#define MAX_6U 63
#define SYMBOL_SIZE 32
#define MNEMONIC_LENGTH 10

#define TEXT_START 0x00400000
#define DATA_START 0x10010000

enum Segment { TEXT, DATA, UNDEF };

enum Binding { LOCAL, GLOBAL };

enum RelocType {
    R_32,
    R_26,
    R_PC16,
    R_HI16,
    R_LO16
};

#define REGISTER_COUNT 32
extern const char *REGISTERS[REGISTER_COUNT];

// Used by both object and executable files
struct FileHeader {
    uint32_t text_size; // Text segment, in bytes
    uint32_t data_size; // Data segment, in bytes
    // Note that the size of the relocation table and symbol tables are not in the main header but the start of their respective sections
    uint32_t entry;     // Used by executable
};

/* === FILE I/O === */

// Writes 8 bits to a file; returns success
int write_byte(FILE *file, uint8_t byte);

// Writes 32 bits to a file; returns success
int write_word(FILE *file, uint32_t word);

// Writes 16 bits to a file; returns success
int write_half(FILE *file, uint16_t half);

// Writes a given number of bytes from a string to a file; returns success
int write_string(FILE *file, const char *str, uint32_t len);

// Reads the next 8 bits from a file
uint8_t read_byte(FILE *file);

// Reads the next 32 bits from a file
uint32_t read_word(FILE *file);

/* === ERROR HANDLING ===
Relies on a global ErrorHandler variable
When a function encounters an error, it calls raise_error() to print the error message.
The program should then terminate.

There are two kinds of errors:
General errors are errors in the execution of the assembler, namely file i/o and memory errors.
Assembler errors are errors in the input, such as invalid syntax.

There's probably a better way to do this.
*/

typedef enum {

    NOERR,

    // General errors
    FILE_IO,     // Failure to open, write, or read a file (object = filename)
    MEM,         // Memory error

    // Assembler errors
    TOKEN_ERR,   // Unrecognized token (object = token)
    SYMBOL_INV,  // Invalid symbol definition (object = symbol)
    ARG_INV,     // Invalid argument (object = argument)
    ARGS_INV,    // Instruction given invalid arguments (no object)
    ST_SIZE_ERR, // Too many symbols in symbol table (object = symbol)
    DUPL_DEF,    // Token defined multiple times (object = token)
    SIZE_ERR,    // Token too large (object = token)

} errcode;

typedef struct {
    const char *file; // What program raised the error
    errcode err_code; // Error code
    const char * err_obj;   // Error object
    const Line *line; // For assembler errors, line in the input file
} ErrorHandler;

extern ErrorHandler ERROR_HANDLER;

void raise_error(errcode, const char *, const char *);

void error_context(const char *);

void error(void);

void general_error(errcode, const char * file, const char * object);

void assembler_error(errcode, const Line *line, const char * object);

/* === OTHER === */

unsigned long hash_key(const char *key, size_t table_size);

enum ImmType {
    SYMBOL,
    NUM,

    // when first pass sees a base offset address of the form IMM(REG),
    // it saves it as a symbol and the second pass later parses it
    // when parse_imm sees a parenthesis, it assumes it is a reg_offset
    // i know this is a terrible solution
    REG_OFFSET,

    NONE,
};

typedef struct {
    enum ImmType type;
    union {
        int32_t intValue;
        char symbol[SYMBOL_SIZE];
    };
    unsigned char rgstr; // used by REG_OFFSET type
    unsigned char modifier; // 0 = none, 1 = hi, 2 = lo, 254 = macro argument, 255 = failure to parse
} Immediate;

// Parses the string into an Immediate structure
Immediate parse_imm(const char *str);

size_t read_escape_sequence(const char *inp, char *res);

extern char * TOKENIZE_START;

char * tokenize(char *str, char delim);

char * read_string(char *dst, size_t *dst_size, char *token, int* len);

unsigned char get_register(const char *token);

void debug_binary(const char *name);

int is_symbol(char c);

#endif //MIPS_ASSEMBLER_UTILS_H