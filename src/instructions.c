#include "instructions.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

/* Instructions

This handles the representation of the MIPS instruction set
as well as converting the intermediate representation Instruction structures to machine code
*/

#define INSTRUCTION_COUNT 63
#define INSTRUCTION_TABLE_SIZE 256 // SHOULD BE DECENTLY LARGER THAN INSTRUCTION COUNT

// ADD NEW INSTRUCTIONS HERE
static const InstrDesc instr_table[INSTRUCTION_COUNT] = {
    // MNEMONIC, OPCODE, FUNCT, FORMAT, REGISTER ORDER

    // MAIN ENCODING
    { "beq",   0x04,  -1, I, {0,1,-1} },    // Conditional branches
    { "bne",   0x05,  -1, I, {0,1,-1} },
    { "blez",  0x06,  -1, I, {0,-1,-1} },
    { "bgtz",  0x07,  -1, I, {0,-1,-1} },
    { "addi",  0x08, -1, I, {1,0,-1} },     // Traditional O-type, i.e. R[rt] = f(R[rs])
    { "addiu", 0x09, -1, I, {1,0,-1} },
    { "slti",  0x0a, -1, I, {1,0,-1} },
    { "sltiu", 0x0b, -1, I, {1,0,-1} },
    { "andi",  0x0c, -1, I, {1,0,-1} },
    { "ori",   0x0d, -1, I, {1,0,-1} },
    { "lui",   0x0f, -1, I, {1,-1,-1} },
    { "lb",    0x20, -1, I, {1,-1,-1} },    // Memory instructions
    { "lh",    0x21, -1, I, {1,-1,-1} },
    { "lwl",   0x22, -1, I, {1,-1,-1} },
    { "lw",    0x23, -1, I, {1,-1,-1} },
    { "lbu",   0x24, -1, I, {1,-1,-1} },
    { "lhu",   0x25, -1, I, {1,-1,-1} },
    { "lwr",   0x26, -1, I, {1,-1,-1} },
    { "sb",    0x28, -1, I, {1,-1,-1} },
    { "sh",    0x29, -1, I, {1,-1,-1} },
    { "swl",   0x2a, -1, I, {1,-1,-1} },
    { "sw",    0x2b, -1, I, {1,-1,-1} },
    { "swr",   0x2e, -1, I, {1,-1,-1} },
    { "j",     0x02,  -1, J, {-1,-1,-1} },  // J TYPE
    { "jal",   0x03,  -1, J, {-1,-1,-1} },
    { "bc",    0x32,  -1, J, {-1,-1,-1} },

    // SPECIAL ENCODING
    { "sll",        0x00, 0x00, R, {2,1,-1}     },  // R TYPE
    { "nop",        0x00, 0x00, R, {-1,-1,-1}   },
    { "srl",        0x00, 0x02, R, {2,1,-1}     },
    { "sra",        0x00, 0x03, R, {2,1,-1}     },
    { "sllv",       0x00, 0x04, R, {2,1,0}      },
    { "srlv",       0x00, 0x04, R, {2,1,0}      },
    { "srav",       0x00, 0x07, R, {2,1,0}      },
    { "jr",         0x00, 0x08, R, {0,-1,-1}    },
    { "jalr",       0x00, 0x09, R, {2,0,1}      },
    { "movz",       0x00, 0x0a, R, {2,0,1}      },
    { "movn",       0x00, 0x0b, R, {2,0,1}      },
    { "syscall",    0x00, 0x0c, R, {-1,-1,-1}   },
    { "break",      0x00, 0x0d, R, {-1,-1,-1}   },
    { "mfhi",       0x00, 0x10, R, {2,-1,-1}    },
    { "mflo",       0x00, 0x12, R, {2,-1,-1}    },
    { "mult",       0x00, 0x18, R, {0,1,-1}     },
    { "multu",      0x00, 0x19, R, {0,1,-1}     },
    { "div",        0x00, 0x1a, R, {0,1,-1}     },
    { "divu",       0x00, 0x1b, R, {0,1,-1}     },
    { "add",        0x00, 0x20, R, {2,0,1}      },
    { "addu",       0x00, 0x21, R, {2,0,1}      },
    { "sub",        0x00, 0x22, R, {2,0,1}      },
    { "subu",       0x00, 0x23, R, {2,0,1}      },
    { "and",        0x00, 0x24, R, {2,0,1}      },
    { "or",         0x00, 0x25, R, {2,0,1}      },
    { "xor",        0x00, 0x26, R, {2,0,1}      },
    { "nor",        0x00, 0x27, R, {2,0,1}      },
    { "slt",        0x00, 0x2a, R, {2,0,1}      },
    { "sltu",       0x00, 0x2b, R, {2,0,1}      },
    { "tge",        0x00, 0x30, R, {0,1,-1}     },
    { "tgeu",       0x00, 0x31, R, {0,1,-1}     },
    { "tlt",        0x00, 0x32, R, {0,1,-1}     },
    { "tltu",       0x00, 0x33, R, {0,1,-1}     },
    { "teq",        0x00, 0x34, R, {0,1,-1}     },
    { "seleqz",     0x00, 0x35, R, {2,0,1}      },
    { "tne",        0x00, 0x36, R, {0,1,-1}     },
    { "selnez",     0x00, 0x37, R, {2,0,1}      },
};

// Allocate memory and initialize hash table
int it_init(InstructionTable *table) {
    table->buckets = malloc(INSTRUCTION_TABLE_SIZE * sizeof(ITBucket));
    if (table->buckets == NULL) {
        raise_error(MEM, NULL, __FILE__);
        return 0;
    }

    table->size = 0;

    // Fill with empty buckets
    for (int i = 0; i < INSTRUCTION_TABLE_SIZE; i++) {
        const ITBucket bucket = {.inUse = 0};
        table->buckets[i] = bucket;
    }

    return 1;
}

// Initializes and populates the hash table
int it_create(InstructionTable *table) {
    if (it_init(table) == 0) return 0;

    if (table->buckets == NULL) return 0;


    // Populate from instr_table
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        it_insert(table, instr_table[i]);
    }

    return 1;
}

// Inserts the InstrDesc into the table using the mnemonic as the key
int it_insert(InstructionTable * table, const InstrDesc desc) {

    unsigned long index = hash_key(desc.mnemonic, INSTRUCTION_TABLE_SIZE); // Get index from key (mnemonic)

    while (table->buckets[index].inUse) {
        index = (index + 1) % INSTRUCTION_TABLE_SIZE; // Increment until we find a spot
    }

    // Copy over to hash map
    table->buckets[index].inUse = 1;
    table->buckets[index].item = desc;
    table->size++;

    return 1;
}

// Returns a pointer to the instruction description for the given mnemonic
InstrDesc *it_lookup(const InstructionTable *table, const char *mnemonic) {
    unsigned long index = hash_key(mnemonic, INSTRUCTION_TABLE_SIZE);
    // Compare with mnemonic at index, increment until we find it
    if (!table->buckets[index].inUse) {
        raise_error(TOKEN_ERR, mnemonic, __FILE__);
        return NULL;
    }
    int indices_searched = 0;
    while (strcmp(mnemonic, table->buckets[index].item.mnemonic) != 0) {
        index = (index + 1) % INSTRUCTION_TABLE_SIZE;
        indices_searched++;
        if (indices_searched >= INSTRUCTION_TABLE_SIZE) {
            raise_error(TOKEN_ERR, mnemonic, __FILE__);
            return NULL;
        }
        if (!table->buckets[index].inUse) {
            raise_error(TOKEN_ERR, mnemonic, __FILE__);
            return NULL;
        }
    }

    return &table->buckets[index].item;
}

// Frees resources
void it_destroy(const InstructionTable *table) {
    free(table->buckets);
}


// A convoluted function: expects as `in` a list of register numbers, and `order` the positions these correspond to.
// See instructions.h for the order.
// Writes the registers in the correct order to the buffer `out`.
int get_registers(unsigned int *out, const unsigned char *in, const int *order) {
    out[0] = 0;
    out[1] = 0;
    out[2] = 0;
    for (int i = 0; i < 3; i++) {
        const int o = order[i]; // which slot (rs 0, rt 1, rd 2)?
        const int r = in[i]; // which register (0-31)?

        if (o == -1) { // If slot not used
            if (r == 255) continue; // And no register given, skip
            return 0;
        }
        if (r == 255) return 0; // No register given but slot used

        out[o] = r;
    }
    return 1;
}

uint32_t convert_rtype(Instruction instruction, const InstrDesc *desc) {
    if (desc->format != R) {
        raise_error(NOERR, NULL, __FILE__);
        return -1;
    }

    // Get fields
    unsigned int opcode = desc->opcode;

    // Get registers
    unsigned int regs[3] = {0,0,0};
    if (get_registers(regs, instruction.registers, desc->register_order) == 0) {
        raise_error(ARGS_INV, NULL, __FILE__);
        return -1;
    }

    const unsigned int funct = desc->funct;

    // Save the immediate field in shamt
    unsigned int shamt = 0;
    if (instruction.imm.type == NUM) {

        // The immediate is usually shamt. The special cases are the trap and break instructions
        // which have different encodings.

        if ((funct >= 0x30 && funct <= 0x34) || funct == 0x36) { // Trap instruction
            if (instruction.imm.intValue > 511 || instruction.imm.intValue < -512) { // should fit in 10 bits
                raise_error(ARGS_INV, NULL, __FILE__);
                return -1;
            }
            opcode <<= 26;
            regs[0] <<= 21;
            regs[1] <<= 16;
            instruction.imm.intValue <<= 6;
            return opcode | regs[0] | regs[1] | instruction.imm.intValue | funct;
        }

        if (funct == 0x0d) { // BREAK instruction
            if (instruction.imm.intValue > 524287 || instruction.imm.intValue < -524288) { // should fit in 20 bits
                raise_error(ARGS_INV, NULL, __FILE__);
                return -1;
            }
            opcode <<= 26;
            instruction.imm.intValue <<= 6;
            return opcode | instruction.imm.intValue | funct;
        }

        /* From the documentation regarding shamt:
            "   If src2 (or the immediate value) is greater than
                31 or less than 0, src1 shifts by src2 MOD 32.  "
        Note this allows the user to provide a 32-bit signed immediate
         */
        shamt = (instruction.imm.intValue % 32 + 32) % 32;
    } else if (instruction.imm.type != NONE) {
        raise_error(ARG_INV, instruction.imm.symbol, __FILE__);
        return -1;
    }

    // Ensure range
    if (regs[0] > MAX_5U || regs[1] > MAX_5U || regs[2] > MAX_5U || shamt > MAX_5U || funct > MAX_6U) {
        raise_error(ARGS_INV, NULL, __FILE__);
        return -1;
    }

    // Shift each field into position
    opcode  <<= 26;
    regs[0] <<= 21; // rs
    regs[1] <<= 16; // rt
    regs[2] <<= 11; // rd
    shamt   <<=  6;
    return opcode | regs[0] | regs[1] | regs[2] | shamt | funct;
}

uint32_t convert_itype(const Instruction instruction, SymbolTable *symbol_table, RelocationTable *reloc_table, const InstrDesc *desc, const uint32_t current_offset) {
    if (desc->format != I) {
        raise_error(NOERR, NULL, __FILE__);
        return -1;
    }

    int opcode = desc->opcode;

    /* I-type instructions are conveniently organized:

     * conditional branches: opcodes 0x4 - 0x7
     * traditional operations (i.e. rt = f(rs)): opcodes 0x8 - 0xF
     * memory instructions: opcodes 0x20 - 0x30
     The immediates for these instructions are handled differently.
    */

    unsigned int regs[3] = {0,0,0};
    if (get_registers(regs, instruction.registers, desc->register_order) == 0) {
        raise_error(ARGS_INV, NULL, __FILE__);
        return -1;
    }
    if (regs[2] != 0) { // Three registers given to I-type instruction
        raise_error(ARGS_INV, NULL, __FILE__);
        return -1;
    }

    // Get immediate
    uint32_t imm = 0;
    if (opcode >= 0x4 && opcode <= 0x7) { // Conditional branch
        if (instruction.imm.type != SYMBOL) {
            raise_error(ARGS_INV, NULL, __FILE__);
            return -1;
        }

        if (instruction.imm.modifier != 0) {
            raise_error(ARGS_INV, NULL, __FILE__);
            return -1;
        }

        Symbol *s = st_get_symbol_safe(symbol_table, instruction.imm.symbol);
        if (s == NULL) return -1;
        RelocationEntry reloc;
        if (re_init(&reloc, current_offset, TEXT, R_PC16, s->name) == 0) return -1;
        rt_add(reloc_table, reloc);

        imm = 0;
    }
    else if (opcode >= 0x8 && opcode <= 0xf) { // Traditional operation
        // Get numerical immediate
        if (instruction.imm.type != SYMBOL && instruction.imm.type != NUM) {
            raise_error(ARGS_INV, NULL, __FILE__);
            return -1;
        }

        // hi and lo bits of a symbol
        if (instruction.imm.type == SYMBOL) {
            Symbol *s = st_get_symbol_safe(symbol_table, instruction.imm.symbol);
            if (s == NULL) return -1;
            RelocationEntry reloc;
            imm = 0;

            switch (instruction.imm.modifier) {
                case 1: // R_HI16
                    if (re_init(&reloc, current_offset, TEXT, R_HI16, s->name) == 0) return -1;
                    break;
                case 2: // R_LO16
                    if (re_init(&reloc, current_offset, TEXT, R_LO16, s->name) == 0) return -1;
                    break;
                default:
                    raise_error(ARG_INV, instruction.imm.symbol, __FILE__);
                    return -1;
            }
            rt_add(reloc_table, reloc);
        }
        else {
            imm = instruction.imm.intValue;
        }

        /* NOTE:
        This allows any immediate value that fits within 16 bits. That is, -32768 to 65535. Different
        instructions have different sign conventions, and this assembler does not check the convention for each
        instruction. This means, for example, the user could input a very small negative number that would get
        interpreted as a very large positive number by the processor.
        */

        // Make sure immediate fits in 16-bits; checks if the upper 17-bits are all 1 or if the upper 16 are all 0
        if ((imm & 0xFFFF8000) != 0xFFFF8000 && (imm & 0xFFFF0000) != 0x00000000) {
            raise_error(ARGS_INV, NULL, __FILE__);
            return -1;
        }

        // Cast to 16 bits
        const int16_t signed_immediate = (int16_t) imm;

        // Convert to unsigned 32-bits
        imm = (uint32_t) signed_immediate & 0x0000FFFF;
    }
    else if (opcode >= 0x20 && opcode <= 0x2e) { // Memory instruction
        // Get rs and immediate from instruction.immediate
        // Address is in the form imm(rs) or (rs), i.e., 0x542($t2). If no number given, 0 is used

        if (instruction.imm.type != REG_OFFSET) {
            raise_error(ARGS_INV, NULL, __FILE__);
            return -1;
        }

        regs[0] = instruction.imm.rgstr;
        switch (instruction.imm.modifier) {
            case 0:
                // numerical value
                if (instruction.imm.intValue > INT16_MAX || instruction.imm.intValue < INT16_MIN) {
                    raise_error(ARGS_INV, NULL, __FILE__);
                    return -1;
                }
                imm = instruction.imm.intValue & 0x0000FFFF;
                break;
            case 1: // hi symbol
            case 2: // lo symbol
                imm = 0;
                RelocationEntry reloc;
                Symbol *s = st_get_symbol_safe(symbol_table, instruction.imm.symbol);
                if (s == NULL) return -1;
                if (instruction.imm.modifier == 1) {if (re_init(&reloc, current_offset, TEXT, R_HI16, s->name) == 0) return -1;}
                else if (instruction.imm.modifier == 2) {if (re_init(&reloc, current_offset, TEXT, R_LO16, s->name) == 0) return -1;}
                else {
                    raise_error(ARG_INV, instruction.imm.symbol, __FILE__);
                    return -1;
                }
                rt_add(reloc_table, reloc);
                break;
            default:
                raise_error(ARGS_INV, NULL, __FILE__);
                return -1;
        }
    }

    // Shift each field into position
    opcode  <<= 26;
    regs[0] <<= 21;
    regs[1] <<= 16;
    return opcode | regs[0] | regs[1] | imm;
}

uint32_t convert_jtype(const Instruction instruction, const SymbolTable *symbol_table, RelocationTable *reloc_table, const InstrDesc *desc, const uint32_t current_offset) {
    if (desc->format != J) {
        raise_error(NOERR, NULL, __FILE__);
        return -1;
    }

    int opcode = desc->opcode;

    /* J-type instuctions use pseudo-direct addresses
    Given a symbol, take the middle 26-bits, dropping the 4 MSBs and 2 LSBs
    The processor later converts this to an address using the 4 MSBs of the PC
    The 4 MSBs of the PC must be equal to those of the address
    */

    if (instruction.imm.type != SYMBOL) {
        raise_error(ARGS_INV, NULL, __FILE__);
        return -1;
    }

    // J-type instructions require R_26 relocation
    const Symbol *s = st_get_symbol_safe(symbol_table, instruction.imm.symbol);
    if (s == NULL) return -1;
    RelocationEntry reloc;
    if (re_init(&reloc, current_offset, TEXT, R_26, s->name) == 0) return -1;
    rt_add(reloc_table, reloc);

    opcode <<= 26;
    return opcode;

}
