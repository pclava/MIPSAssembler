#ifndef MIPS_ASSEMBLER_PREPROCESS_H
#define MIPS_ASSEMBLER_PREPROCESS_H

#include <stdio.h>

#include "text.h"

FILE * open_file(const char *path);

int preprocess(FILE *inp, Text *text);

#endif //MIPS_ASSEMBLER_PREPROCESS_H