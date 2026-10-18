#include "instructions.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>

/* Instructions

This handles the representation of the MIPS instruction set
as well as converting the intermediate representation Instruction structures to machine code
*/

#define COP0 0x10       // opcode for coprocessor 0 instructions
#define C0 0b10000      // RS field for some COP0 instructions
#define SPECIAL 0x00    // opcode for most r-type instructions
#define MFMC0 0b01011

#define INSTRUCTION_COUNT 74
#define INSTRUCTION_TABLE_SIZE 256 // SHOULD BE DECENTLY LARGER THAN INSTRUCTION COUNT

static const InstrDesc instr_table[INSTRUCTION_COUNT] = {
    // MMEMONIC, ARGC, OPCODE, RS, ARGS, DEFAULTS, FUNCT

    // Main encoding
    { "beq",    3, 0x04, {RS, RT, PC16},  {}, -1},
    { "bne",    3, 0x05, {RS, RT, PC16},  {}, -1},
    { "blez",   2, 0x06, {RS, PC16},      {}, -1},
    { "bgtz",   2, 0x07, {RS, PC16},      {}, -1},
    { "addi",   3, 0x08, {RT, RS, IMM16},  {}, -1},
    { "addiu",  3, 0x09, {RT, RS, IMM16},  {}, -1},
    { "slti",   3, 0x0a, {RT, RS, IMM16},  {}, -1},
    { "sltiu",  3, 0x0b, {RT, RS, IMM16},  {}, -1},
    { "andi",   3, 0x0c, {RT, RS, IMM16},  {}, -1},
    { "ori",    3, 0x0d, {RT, RS, IMM16},  {}, -1},
    { "xori",   3, 0x0e, {RT, RS, IMM16},  {}, -1},
    { "lui",    2, 0x0f, {RT, IMM16},      {}, -1},
    { "lb",     2, 0x20, {RT, REGOFF},     {}, -1},
    { "lh",     2, 0x21, {RT, REGOFF},     {}, -1},
    { "lwl",    2, 0x22, {RT, REGOFF},     {}, -1},
    { "lw",     2, 0x23, {RT, REGOFF},     {}, -1},
    { "lbu",    2, 0x24, {RT, REGOFF},     {}, -1},
    { "lhu",    2, 0x25, {RT, REGOFF},     {}, -1},
    { "lwr",    2, 0x26, {RT, REGOFF},     {}, -1},
    { "sb",     2, 0x28, {RT, REGOFF},     {}, -1},
    { "sh",     2, 0x29, {RT, REGOFF},     {}, -1},
    { "swl",    2, 0x2a, {RT, REGOFF},     {}, -1},
    { "sw",     2, 0x2b, {RT, REGOFF},     {}, -1},
    { "sr",     2, 0x2e, {RT, REGOFF},     {}, -1},
    { "j",      1, 0x02, {JADDR},          {}, -1},
    { "jal",    1, 0x03, {JADDR},          {}, -1},

    // SPECIAL encoding
    { "sll",        3, SPECIAL, {RD, RT, SHAMT},   {}, 0x00},
    { "nop",        0, SPECIAL, {},                {}, 0},
    { "srl",        3, SPECIAL, {RD, RT, SHAMT},   {}, 0x02},
    { "sra",        3, SPECIAL, {RD, RT, SHAMT},   {}, 0x03},
    { "sllv",       3, SPECIAL, {RD, RT, RS},      {}, 0x04},
    { "srlv",       3, SPECIAL, {RD, RT, RS},      {}, 0x06},
    { "srav",       3, SPECIAL, {RD, RT, RS},      {}, 0x07},
    { "jr",         1, SPECIAL, {RS},              {}, 0x08},
    { "jalr",       1, SPECIAL, {RS},              {{RD, 31}}, 0x09}, // TODO: check for multiple of the same name, distinguished by argc
    { "jalr",       2, SPECIAL, {RD, RS},          {}, 0x09},
    { "movz",       3, SPECIAL, {RD, RS, RT},      {}, 0x0a},
    { "movn",       3, SPECIAL, {RD, RS, RT},      {}, 0x0b},
    { "syscall",    0, SPECIAL, {},                {}, 0x0c},
    { "break",      0, SPECIAL, {},                {}, 0x0d},
    { "mfhi",       1, SPECIAL, {RD},              {}, 0x10},
    { "mthi",       1, SPECIAL, {RS},              {}, 0x11},
    { "mflo",       1, SPECIAL, {RT},              {}, 0x12},
    { "mtlo",       1, SPECIAL, {RS},              {}, 0x13},
    { "mult",       2, SPECIAL, {RS, RT},          {}, 0x18},
    { "multu",      2, SPECIAL, {RS, RT},          {}, 0x19},
    { "div",        2, SPECIAL, {RS, RT},          {}, 0x1a},
    { "divu",       2, SPECIAL, {RS, RT},          {}, 0x1b},
    { "add",        3, SPECIAL, {RD, RS, RT},      {}, 0x20},
    { "addu",       3, SPECIAL, {RD, RS, RT},      {}, 0x21},
    { "sub",        3, SPECIAL, {RD, RS, RT},      {}, 0x22},
    { "subu",       3, SPECIAL, {RD, RS, RT},      {}, 0x23},
    { "and",        3, SPECIAL, {RD, RS, RT},      {}, 0x24},
    { "or",         3, SPECIAL, {RD, RS, RT},      {}, 0x25},
    { "xor",        3, SPECIAL, {RD, RS, RT},      {}, 0x26},
    { "nor",        3, SPECIAL, {RD, RS, RT},      {}, 0x27},
    { "slt",        3, SPECIAL, {RD, RS, RT},      {}, 0x2a},
    { "sltu",       3, SPECIAL, {RD, RS, RT},      {}, 0x2b},
    { "tge",        2, SPECIAL, {RS, RT},          {}, 0x30},
    { "tgeu",       2, SPECIAL, {RS, RT},          {}, 0x31},
    { "tlt",        2, SPECIAL, {RS, RT},          {}, 0x32},
    { "tltu",       2, SPECIAL, {RS, RT},          {}, 0x33},
    { "teq",        2, SPECIAL, {RS, RT},          {}, 0x34},
    { "tne",        2, SPECIAL, {RS, RT},          {}, 0x36},

    // COP0 encoding
    {"eret",    0, COP0, {},       {{RS, C0}}, 0x18},
    {"wait",    0, COP0, {},       {{RS, C0}}, 0x20},
    {"mfc0",    2, COP0, {RT, RD}, {}, 0x00},
    {"mfc0",    3, COP0, {RT, RD, SEL}, {}, 0x00},
    {"mtc0",    2, COP0, {RT, RD}, {{RS, 0b100}}, 0x00},
    {"mtc0",    3, COP0, {RT, RD, SEL}, {{RS, 0b100}}, 0x00},
    {"ei",      0, COP0, {},       {{RS, MFMC0}, {RD, 0b01100}}, 0b100000},
    {"ei",      1, COP0, {RT},     {{RS, MFMC0}, {RD, 0b01100}}, 0b100000},
    {"di",      0, COP0, {},       {{RS, MFMC0}}, 0b100000},
    {"di",      1, COP0, {RT},     {{RS, MFMC0}}, 0b100000},

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
    try(it_init(table), 0);

    if (table->buckets == NULL) return 0;

    // Populate from instr_table
    for (int i = 0; i < INSTRUCTION_COUNT; i++) {
        try(it_insert(table, instr_table[i]), 0);
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
InstrDesc *it_lookup(const InstructionTable *table, const char *mnemonic, const int argc) {
    unsigned long index = hash_key(mnemonic, INSTRUCTION_TABLE_SIZE);
    // Compare with mnemonic at index, increment until we find it
    if (!table->buckets[index].inUse) {
        raise_error(TOKEN_ERR, mnemonic, __FILE__);
        return NULL;
    }

    int indices_searched = 0;

    // Check mnemonic AND argc
    while (strcmp(mnemonic, table->buckets[index].item.mnemonic) != 0 || table->buckets[index].item.argc != argc) {
        index = (index + 1) % INSTRUCTION_TABLE_SIZE;
        indices_searched++;
        if (indices_searched >= INSTRUCTION_TABLE_SIZE) {
            raise_error(TOKEN_ERR, mnemonic, __FILE__);
            error_context("could not find corresponding instruction");
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

uint32_t convert_instruction(Instruction instruction, InstructionTable *instruction_table, RelocationTable *reloc_table, uint32_t current_addr, Segment current_segment) {
    // Find description
    const InstrDesc *desc = it_lookup(instruction_table, instruction.mnemonic, instruction.argc);
    if (desc == NULL) return -1;

    uint32_t final = 0;

    // Fill in opcode, funct (if not -1)
    final |= (desc->opcode << 26);
    if (desc->funct != -1) final |= (desc->funct & 0b111111);

    // For each arg, fill in
    for (int i = 0; i < instruction.argc; i++) {
        Argument userarg = instruction.args[i];
        ArgPosition position = desc->args[i];

        Symbol *s;
        RelocationEntry reloc;

        if (position == NOARG) {
            // Too many arguments were given (this shouldn't happen because we compare argcs)
            goto bad;
        }

        // Argument should be register
        if (position >= RS && position <= RD) {
            if (userarg.type != REGISTER) goto bad;
            unsigned char reg = userarg.reg;
            reg &= 0b11111;

            // Place user value into position
            switch (position) {
                case RS:
                    final |= (reg << 21);
                    break;
                case RT:
                    final |= (reg << 16);
                    break;
                case RD:
                    final |= (reg << 11);
                    break;
                default:
                    goto bad;
            }
        }

        // Argument should be immediate
        else if (position >= SHAMT) {
            if (userarg.type != IMMEDIATE) goto bad;
            Immediate imm = userarg.imm;
            uint32_t v;

            switch (position) {
                case SHAMT:
                    if (imm.type != NUM) goto bad;
                    /* From the documentation regarding shamt:
                        "   If src2 (or the immediate value) is greater than
                            31 or less than 0, src1 shifts by src2 MOD 32.  "
                    Note this allows the user to provide a 32-bit signed immediate
                     */
                    v = (imm.intValue % 32 + 32) % 32;
                    final |= (v << 6);
                    break;
                case JADDR:
                    if (imm.type != SYMBOL) goto bad;

                    /* J-type instuctions use pseudo-direct addresses
                    Given a symbol, take the middle 26-bits, dropping the 4 MSBs and 2 LSBs
                    The processor later converts this to an address using the 4 MSBs of the PC
                    The 4 MSBs of the PC must be equal to those of the address
                    */

                    // J-type instructions require R_26 relocation
                    s = imm.symbol;
                    if (s == NULL) goto bad;
                    try(re_init(&reloc, s->index, current_addr, current_segment, R_26), -1);
                    rt_add(reloc_table, reloc);

                    break;
                case PC16:
                    if (imm.type != SYMBOL || imm.modifier != 0) goto bad;

                    s = imm.symbol;
                    if (s == NULL) goto bad;
                    try(re_init(&reloc, s->index, current_addr, current_segment, R_PC16), -1);
                    rt_add(reloc_table, reloc);

                    break;
                case IMM16:
                    if (imm.type != SYMBOL && imm.type != NUM) goto bad;

                    if (imm.type == SYMBOL) {
                        s = imm.symbol;
                        if (s == NULL) goto bad;
                        switch (imm.modifier) {
                            case 1: // R_HI16
                                try(re_init(&reloc, s->index, current_addr, current_segment, R_HI16), -1);
                                break;
                            case 2: // R_LO16
                                try(re_init(&reloc, s->index, current_addr, current_segment, R_LO16), -1);
                                break;
                            default:
                                goto bad;
                        }
                        rt_add(reloc_table, reloc);
                    }

                    /* NOTE:
                    This allows any immediate value that fits within 16 bits. That is, -32768 to 65535. Different
                    instructions have different sign conventions, and this assembler does not check the convention for each
                    instruction. This means, for example, the user could input a very small negative number that would get
                    interpreted as a very large positive number by the processor.
                    */

                    else {
                        // imm = instruction.imm.intValue;
                        switch (imm.modifier) {
                            case 0:
                                v = imm.intValue;
                                break;
                            case 1:
                                v = imm.intValue >> 16;
                                break;
                            case 2:
                                v = imm.intValue & 0xFFFF;
                                break;
                            default:
                                raise_error(ARGS_INV, NULL, __FILE__);
                                return -1;
                        }

                        // Make sure immediate fits in 16-bits; checks if the upper 17-bits are all 1 or if the upper 16 are all 0
                        if ((v & 0xFFFF8000) != 0xFFFF8000 && (v & 0xFFFF0000) != 0x00000000) goto bad;
                        int16_t signed_v = (int16_t) v;
                        v = (uint32_t) signed_v & 0xFFFF;
                        final |= v;
                    }

                    break;
                case REGOFF:
                    if (imm.type != REG_OFFSET) goto bad;

                    // Set RS to register value
                    final |= (imm.rgstr << 21);

                    switch (imm.modifier) {
                        case 0:
                            // Numerical value
                            if (imm.intValue > UINT16_MAX || imm.intValue < INT16_MIN) goto bad;
                            final |= (imm.intValue & 0xFFFF);
                            break;
                        case 1: // hi symbol
                        case 2: // lo symbol
                            s = imm.symbol;
                            if (s == NULL) goto bad;
                            if (imm.modifier == 1) {
                                try(re_init(&reloc, s->index, current_addr, current_segment, R_HI16), -1);
                            }
                            else if (imm.modifier == 2) {
                                try(re_init(&reloc, s->index, current_addr, current_segment, R_LO16), -1);
                            } else goto bad;
                            rt_add(reloc_table, reloc);
                            break;
                        default:
                            goto bad;
                    }

                    break;
                case SEL:
                    if (imm.type != NUM) goto bad;
                    if ((imm.intValue & 0b111) != imm.intValue) goto bad;
                    final |= imm.intValue;

                    break;
                default:
                    return -1;
            }
        }
    }

    // Fill in defaults
    int i = 0;
    while (desc->defaults[i].position != NOARG) {
        const ArgValue arg = desc->defaults[i];
        uint32_t v = (uint32_t) arg.value;
        switch (arg.position) {
            case RS:
                v &= 0b11111;
                final |= (v << 21);
                break;
            case RT:
                v &= 0b11111;
                final |= (v << 16);
                break;
            case RD:
                v &= 0b11111;
                final |= (v << 11);
                break;
            case SHAMT:
                v &= 0b11111;
                final |= (v << 6);
                break;
            case JADDR:
                v &= 0x3FFFFFF;
                final |= v;
                break;
            case IMM16:
            case PC16:
                v &= 0xFFFF;
                final |= v;
                break;
            default: goto bad;
        }
        i++;
    }

    return final;

    bad:
    raise_error(ARGS_INV, NULL, __FILE__);
    return -1;
}