# MIPS Assembler and Linker
## Overview
An assembler and linker for a subset of the MIPS instruction set, written in C. I created this shortly after learning C and MIPS to gain experience with C projects (and projects in general) and improve my skills in C.

There are two executables: an assembler and a linker. The former generates object files which the latter links into an executable.
The executables can be run by my MIPS emulator at https://github.com/pclava/MIPSEmulator.

The object and executable files use the MOF format specified in `include/mof.h`. This is a minimal executable and linkable format written for this assembler and linker.

The particular details of the MIPS instruction set were sourced from _MIPS Assembly Language Programmer's Guide_ (Silicon Graphics, 1992) and _The MIPS32® Instruction Set Manual_ (MIPS Technologies, 2016). 

## Usage
To install, download one of the available binaries under **Releases**, or build the assembler yourself by running `make` from the main directory.

The assembler takes as arguments the source assembly files and outputs object files with the same name and the `.o` file extension:

`$ MIPSAssembler <files>`

The linker takes as arguments the target object files as well as optional flags:

`$ MIPSLinker <options> <files>`

The available options are:

- `-c` deletes ("cleans") the object files after linking (be careful!)
- `-d <path>` dumps the global symbol table to a text file for debugging purposes 
- `-o <path>` writes the executable to the given path. if not provided, writes to `a.out`
- `-s <symbol>` sets program entry at the given symbol. if not provided, sets to the first instruction (address 0x00400000)
- `-ls` links __start.o and sets program entry to `__start`. overrides the `-s` flag if put after
 

### Example
To create an executable from two files, `foo.asm` and `bar.asm`, first assemble:

`$ MIPSAssembler foo.asm bar.asm`

This will generate two object files, `foo.o` and `bar.o`. To link, the minimal command is:

`$ MIPSLinker foo.o bar.o`

This outputs an executable `a.out`. However, one could also provide any of the above flags. For example,

`$ MIPSLinker -o baz.out -ls -d symbols.txt -c foo.o bar.o`

This links `__start.o` and the given object files, outputs an executable `baz.out` with program entry at `__start`, dumps the global symbol table at in `symbols.txt`, and deletes the two object files after completion. 

**NOTE**: It is important that `pseudo.asm` and `__start.o` be in the same directory as the executable. Run `MIPSAssembler __start.asm` to generate `__start.o`. Note that `__start.asm` tries to jump to a global symbol `main`. If the code lacks such a symbol, the linking will fail.
Additionally, the assembler currently only works with files in the current directory or in a sub-directory.

## Support
The assembler does not support the entire MIPS instruction set. It namely does not support floating-point instructions. Consult `src/instructions.c` for the full list.

---
Every line of code in this repository was written by a human.






