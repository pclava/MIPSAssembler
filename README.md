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
The `build` script takes any number of files as input and assembles and links them.
Unless an output path is specified, the program outputs the resulting executable as `a.out` in its current directory.

`build` supports the following options:
- `-o [path]`
Writes the executable in `path`, instead of `a.out`. Incompatible with the `-c` option.
- `-c`
Stops the assembler before linking. Only outputs object files, with the same names as the input files.
- `-e.` and `-e [symbol]`   
By default, when linking object files, the assembler also links `_start.o`, which calls the `main` function. An error will be raised if `main` does not exist.
To prevent the assembler from linking `_start.o`, the `-e [symbol]` flag sets the program entry to `symbol`, and the `-e.` flag sets the program entry to the first instruction in the text segment (0x00400000).

### Examples
- `$ ./build examples/helloworld.asm`
assembles `helloworld.asm`, links with `_start.o`, and writes the result to `a.out`.


- `$ ./build -e. examples/helloworld.asm`
only assembles and links `helloworld.asm`, with the program entry set to 0x00400000.


- `$ ./build -c _start.asm`
assembles `_start.asm` into `_start.o`.


- `$ ./build -o fibonacci.out examples/fibonacci/functs.asm examples/fibonacci/fibonacci.asm`
assembles and links `functs.asm`, `fibonacci.asm`, and `_start.o`, writing the result to `fibonacci.out`.

## Support
The assembler does not support the entire MIPS instruction set. It does not support floating-point instructions. See `src/instructions.c` for the full list.
  
---
Every line of code in this repository was written by a human.






