# MIPS Assembler and Linker
## Overview
An assembler and linker for a subset of the MIPS instruction set, written in C. I created this shortly after learning C and MIPS to gain experience with C projects (and projects in general) and improve my skills in C.
The program consists of a preprocessor, assembler, and linker. It can assemble and link multiple files into an executable, or just assemble them into individual object files.

The executable has the following format:
```
* Header
  - 4 bytes: Text size (in bytes)
  - 4 bytes: Data size (in bytes)
  - 4 bytes: Program entry (32-bit memory address)
* Text segment
* Data segment
```

Object files have the following format:
```
* Header
  - 4 bytes: Text size (in bytes)
  - 4 bytes: Data size (in bytes)
  - 4 bytes: Program entry (32-bit memory address) (ignored)
* Text segment
* Data segment
* Relocation table
  - 4 bytes: number of entries
  - ...
* Symbol table
  - 4 bytes: number of entries
  - ...
```

The particular details of the MIPS instruction set were sourced from _MIPS Assembly Language Programmer's Guide_ (Silicon Graphics, 1992). 
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

For example, to build an executable `foo.out` from the source files `bar.asm` and `baz.asm`, one could run:

`mips_assembler -o foo.out bar.asm baz.asm`

**NOTE**: It is important that `pseudo.asm` and `__start.o` be in the same directory as the executable.
Additionally, he assembler currently only works with files in the current directory or in a sub-directory.

## Support
The assembler does not support the entire MIPS instruction set. It namely does not support floating-point instructions. Consult `src/instructions.c` for the full list.

---
Every line of code in this repository was written by a human.






