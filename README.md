# MIPS Assembler and Linker
## Overview
An assembler and linker for a subset of the MIPS instruction set, written in C. I created this shortly after learning C and MIPS to gain experience with C projects (and projects in general) and improve my skills in C.
The program consists of a preprocessor, assembler, and linker. It can assemble and link multiple files into an executable, or just assemble them into individual object files.
The executables can be run by my MIPS emulator at https://github.com/pclava/MIPSEmulator.

The object and executable files use the MOF format specified in `include/mof.h`. This is a minimal executable and linkable format written for this assembler and linker.

The particular details of the MIPS instruction set were sourced from _MIPS Assembly Language Programmer's Guide_ (Silicon Graphics, 1992) and _The MIPS32® Instruction Set Manual_ (MIPS Technologies, 2016). 
Some example assembly files and their outputs can be found in `examples/`.

## Usage
To install, download one of the available binaries under **Releases**, or build the assembler yourself by running `make` from the main directory.

`$ mips_assembler <options> <source files>`

Note the files are linked **in the order they are listed**.

The options include:
- `-o <file>` : writes the executable to the given file (if not provided, outputs to a.out). Ignored if -c flag is provided
- `-c` : stops after assembler, outputs unlinked object files for each source
- `-e.` : begin execution at the first instruction
- `-e <symbol>` : begin execution at the given global symbol (if not provided, linker links __start and begins execution there)

For example, to build an executable `foo.out` from the source files `bar.asm` and `baz.asm` (while linking `__start`), one could run:

`MIPSAssembler -o foo.out bar.asm baz.asm`

**NOTE**: It is important that `pseudo.asm` and `__start.o` be in the same directory as the executable.
Additionally, the assembler currently only works with files in the current directory or in a sub-directory.

## Support
The assembler does not support the entire MIPS instruction set. It namely does not support floating-point instructions. Consult `src/instructions.c` for the full list.

---
Every line of code in this repository was written by a human.






