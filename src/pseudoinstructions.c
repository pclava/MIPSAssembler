#include "pseudoinstructions.h"

#include <ctype.h>

#include "instruction_parser.h"
#include "utils.h"
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* Pseudoinstructions

This handles the conversion of pseudoinstructions to their real equivalents.
The functions below take the pseudoinstruction in the form of an Instruction structure
and add its real equivalents directly to the InstructionList
*/

int mt_init(MacroTable *table) {
    table->buckets = malloc(MACRO_TABLE_LENGTH * sizeof(MacroBucket));
    if (table->buckets == NULL) {
        raise_error(MEM, NULL, __FILE__);
        return 0;
    }

    table->size = 0;

    // Fill with empty buckets
    for (int i = 0; i < MACRO_TABLE_LENGTH; i++) {
        const MacroBucket bucket = {.inUse = 0};
        table->buckets[i] = bucket;
    }

    return 1;
}

int mt_add(MacroTable *table, const Macro macro) {
    unsigned long index = hash_key(macro.name, MACRO_TABLE_LENGTH);
    while (table->buckets[index].inUse) {
        if (strcmp(table->buckets[index].macro.name, macro.name) == 0) {
            raise_error(DUPL_DEF, macro.name, __FILE__);
            return 0;
        }
        index = (index + 1) % MACRO_TABLE_LENGTH;
    }

    table->buckets[index].inUse = 1;
    table->buckets[index].macro = macro;
    table->size++;

    return 1;
}

unsigned long mt_exists(const MacroTable *table, const char *name) {
    unsigned long index = hash_key(name, MACRO_TABLE_LENGTH);
    if (!table->buckets[index].inUse) {
        return MACRO_TABLE_LENGTH;
    }
    int indices_searched = 0;
    while (strcmp(name, table->buckets[index].macro.name) != 0) {
        index = (index + 1) % MACRO_TABLE_LENGTH;
        indices_searched++;
        if (indices_searched == MACRO_TABLE_LENGTH) {
            return MACRO_TABLE_LENGTH;
        }
    }
    return index;
}

Macro * mt_get(const MacroTable *table, const char *name) {
    unsigned long index = mt_exists(table, name);
    if (index == MACRO_TABLE_LENGTH) {
        raise_error(TOKEN_ERR, name, __FILE__);
        return NULL;
    }

    return &table->buckets[index].macro;
}

void mt_destroy(const MacroTable *t) {
    free(t->buckets);
}

void mt_debug(const MacroTable *table) {
    for (int i = 0; i < MACRO_TABLE_LENGTH; i++) {
        if (table->buckets[i].inUse) {
            macro_debug(&table->buckets[i].macro);
        }
    }
}

void macro_debug(const Macro *m) {
    printf("macro \"%s\"\n", m->name);
    Line *cur = m->definition_start;
    for (size_t i = 0; i < m->definition_length; i++) {
        printf("%s", cur->text);
        cur = cur->next;
    }
}

// Returns last line (.end_macro ...)
Line *define_macro(Macro *macro, const Line *line) {

    memset(macro, '\0', sizeof(Macro));

    // Copy string to buffer
    char buf[strlen(line->text)+1];
    strcpy(buf,line->text);

    // Tokenize
    char *token = tokenize(buf, ' ');
    if (strcmp(token, ".macro") != 0) return NULL;

    // Get name
    token = tokenize(NULL, ' ');
    if (token == NULL || strlen(token) >= MACRO_SIZE) {
        raise_error(ARGS_INV, NULL, __FILE__);
        return NULL;
    }
    strcpy(macro->name, token);
    // Ensure name (and later arguments) are wholly alphanum
    for (size_t i = 0; i < strlen(macro->name); i++) {
        if (!isalnum(macro->name[i])) {
            raise_error(SYMBOL_INV, macro->name, __FILE__);
            return NULL;
        }
    }

    // Get argument names (up to 32 arguments should be plenty)
    token = tokenize(NULL, ' ');
    size_t argc = 0;
    while (argc < 32) {
        if (token == NULL) break;

        if (strlen(token) >= 32 || token[0] != '%') {
            raise_error(ARG_INV, token, __FILE__);
            return NULL;
        }

        // raise error if using hi and lo operators as argument names
        if (strcmp(token, "%hi") == 0 || strcmp(token, "%lo") == 0) {
            raise_error(ARG_INV, token, __FILE__);
            error_context("Cannot use 'hi' or 'lo' as macro argument");
            return NULL;
        }
        strcpy(macro->args[argc++], token);
        for (size_t i = 1; i < strlen(macro->args[argc-1]); i++) {
            if (!isalnum(macro->args[argc-1][i])) {
                raise_error(SYMBOL_INV, macro->args[argc-1], __FILE__);
                return NULL;
            }
        }

        token = tokenize(NULL, ' ');
    }
    if (argc >= 32) {
        raise_error(ARGS_INV, NULL, __FILE__);
        return NULL;
    }

    // Read remaining lines
    Line *temp = line->next;
    if (temp == NULL) {
        raise_error(ARGS_INV, NULL, __FILE__);
        return NULL;
    }
    macro->definition_start = temp;
    macro->definition_length = 0;
    while (strcmp(temp->text, ".end_macro") != 0) {

        macro->definition_length++;

        temp = temp->next;
        if (temp == NULL) {
            raise_error(ARGS_INV, NULL, __FILE__);
            return NULL;
        }
    }

    return temp;
}

int insert_macro(Text *text_list, const MacroTable *table, const char *name, Line *line) {

    // Get macro
    Macro *macro = mt_get(table, name);
    if (macro == NULL) return 0;

    // Copy text to buffer
    char buf[strlen(line->text)+1];
    strcpy(buf,line->text);

    // Retrieve arguments (up to 32 arguments of 32 characters each)
    char args[MACRO_SIZE][MACRO_SIZE];
    memset(args, '\0', sizeof(args));
    char *token = tokenize(buf, ' ');
    // Iterate until we get back to the name (skipping labels)
    while (strcmp(token, name) != 0) {
        token = tokenize(NULL, ' ');
    }
    // Copy over arguments
    token = tokenize(NULL, ' ');
    size_t argc = 0;
    while (argc < 32) {
        if (token == NULL) break;

        if (strlen(token) >= 32) {
            raise_error(ARG_INV, token, __FILE__);
            return 0;
        }

        strcpy(args[argc++], token);

        token = tokenize(NULL, ' ');
    }
    if (argc >= 32) {
        raise_error(ARGS_INV, NULL, __FILE__);
        return 0;
    }

    // Translate and insert macro
    Line *new_line = macro->definition_start;
    Line *before = line;
    for (size_t i = 0; i < macro->definition_length; i++) {
        // For each line in the definition
        // Initialize a new line
        // Copy over until we see a macro argument, then copy that

        // Initialize new line
        Line to_insert;
        line_init(&to_insert, line->filename);
        to_insert.number = line->number;

        // Read definition line
        char c;
        int index=0;
        while ((c = new_line->text[index++]) != '\0') {
            // Loop until null

            // If percent
            if (c == '%') {

                // Get argument name
                char argbuf[MACRO_SIZE];
                memset(argbuf, '\0', sizeof(argbuf));
                argbuf[0] = '%';
                int j = 1;
                while (is_symbol(c = new_line->text[index++])) {
                    argbuf[j++] = c;
                }
                const char end_char = c;

                // skip the hi and lo operators
                if (strcmp(argbuf, "%lo") == 0) {
                    line_add_char(&to_insert, '%');
                    line_add_char(&to_insert, 'l');
                    line_add_char(&to_insert, 'o');
                    line_add_char(&to_insert, end_char);
                    continue;
                }
                if (strcmp(argbuf, "%hi") == 0) {
                    line_add_char(&to_insert, '%');
                    line_add_char(&to_insert, 'h');
                    line_add_char(&to_insert, 'i');
                    line_add_char(&to_insert, end_char);
                    continue;
                }

                // Find index in macro->args
                j = 0;
                int res = 0;
                while (strcmp(macro->args[j++], argbuf) != 0) {
                    if (j == MACRO_SIZE || macro->args[j-1][0] == '\0') { // if we've checked every argument
                        raise_error(ARG_INV, argbuf, __FILE__);
                        return 0;
                    }
                }
                res = j-1;

                // Real argument is args[res]; copy that
                for (size_t k = 0; k < strlen(args[res]); k++) {
                    line_add_char(&to_insert, args[res][k]);
                }

                // Add space
                line_add_char(&to_insert, end_char);
            }

            // Otherwise insert normally
            else line_add_char(&to_insert, c);
        }
        line_add_char(&to_insert, '\0'); // to be safe

        // Insert into text
        before = text_insert(text_list, to_insert, before);

        new_line = new_line->next;
    }

    // text_debug(text_list);
    return 1;
}

int li(const Instruction instruction, InstructionList* instructions) {
    // li $R IMM
    if (instruction.registers[1] != 255 || instruction.registers[2] != 255 || instruction.imm.type != NUM) {
        raise_error(ARGS_INV, NULL, __FILE__);
        return 0;
    }
    const unsigned char r1 = instruction.registers[0];
    const Immediate imm = instruction.imm;

    Instruction i1;
    i1.line = instruction.line;
    memset(i1.mnemonic, '\0', sizeof(i1.mnemonic));

    // Determine size
    if (instruction.imm.intValue >= SHRT_MIN && instruction.imm.intValue <= SHRT_MAX) {
        // 16-bit
        // addi $R $0 IMM
        strcpy(i1.mnemonic, "addiu");
        i1.registers[0] = r1;
        i1.registers[1] = 0;
        i1.registers[2] = 255;
        i1.imm = imm;
        if (add_instruction(instructions, i1) == 0) {
            return 0;
        }
        return 1;
    }
    // 32-bit
    const int32_t hi = imm.intValue >> 16;     // Sign extended upper 16-bits
    const int32_t lo = imm.intValue & 0x0000FFFF;  // Low 16-bits

    const Immediate hiImm = {NUM, .intValue = hi};
    const Immediate loImm = {NUM, .intValue = lo};

    // lui $1 %hi(IMM)
    strcpy(i1.mnemonic, "lui");
    i1.registers[0] = 1;
    i1.registers[1] = 255;
    i1.registers[2] = 255;
    i1.imm = hiImm;

    // ori $R $1 %lo(IMM)
    Instruction i2;
    memset(i2.mnemonic, '\0', sizeof(i2.mnemonic));
    strcpy(i2.mnemonic, "ori");
    i2.registers[0] = r1;
    i2.registers[1] = 1;
    i2.registers[2] = 255;
    i2.line = instruction.line;
    i2.imm = loImm;

    if (add_instruction(instructions, i1) == 0 || add_instruction(instructions, i2) == 0) {
        return 0;
    }
    return 2;
}

