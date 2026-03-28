#ifndef MIPS_ASSEMBLER_UTILS_H
#define MIPS_ASSEMBLER_UTILS_H
#include "symbol_table.h"
#include "text.h"
#include "mof.h"

#include <stdint.h>
#include <stdio.h>

#define try(expression, ret) if (expression == 0) return ret;

/* === CONSTANTS === */
#define MAX_5U 31
#define MAX_6U 63
#define MNEMONIC_LENGTH 10

#define TEXT_START 0x00400000
#define DATA_START 0x10010000
#define KTEXT_START 0x80000180
#define KDATA_START 0x90000000

#define REGISTER_COUNT 32
extern const char *REGISTERS[REGISTER_COUNT];

extern const char *C0_REGISTERS[REGISTER_COUNT];

#define COMMENT '#'

typedef enum mof_binding Binding;
typedef enum mof_segment Segment;
typedef enum mof_reloctype RelocType;

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
    FILE_IO,        // Failure to open, write, or read a file (object = filename)
    MEM,            // Memory error
    BAD_FILE,       // Invalid file format

    // Assembler errors
    TOKEN_ERR,      // Unrecognized token (object = token)
    SYMBOL_INV,     // Invalid symbol definition (object = symbol name)
    ARG_INV,        // Invalid argument (object = argument)
    ARGS_INV,       // Instruction given invalid arguments (no object)
    DUPL_DEF,       // Token defined multiple times (object = token)
    SIZE_ERR,       // Token too large (object = token)
    INSTR_INV,      // Invalid instruction (object = mnemonic)

} errcode;

typedef struct {
    const char *file; // What program raised the error
    errcode err_code; // Error code
    const char * err_obj;   // Error object
    const Line *line; // For assembler errors, line in the input file
    const char * file_name; // file currently being read
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

typedef struct Immediate Immediate;

// Forward declaration
typedef struct mof_symbol Symbol;
typedef struct SymbolTable SymbolTable;

struct Immediate {
    enum ImmType type;
    union {
        int32_t intValue;
        Symbol *symbol;
    };
    unsigned char rgstr; // used by REG_OFFSET type
    unsigned char modifier; // 0 = none, 1 = hi, 2 = lo, 254 = macro argument, 255 = failure to parse
};

// Parses the string into an Immediate structure. read_escape is whether the function should resolve escape sequences
Immediate parse_imm(const char *str, SymbolTable *symbol_table, int read_escape);

size_t read_escape_sequence(const char *inp, char *res);

extern char * TOKENIZE_START;

char * tokenize(char *str, char delim);

char * read_string(char *dst, size_t *dst_size, char *token, int* len);

unsigned char get_register(const char *token);

void debug_binary(const char *name);

int issymbol(char c);

#endif //MIPS_ASSEMBLER_UTILS_H