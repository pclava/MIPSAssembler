#include "preprocess.h"
#include "utils.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

FILE * open_file(const char *path) {
    FILE *inp = fopen(path, "r");
    if (inp == NULL) {
        general_error(FILE_IO, __FILE__, path);
        return NULL;
    }
    return inp;
}

// Splits a file into lines and writes those to the Text structure
int split(FILE *inp, Text *lines) {
    char buf[512];
    Line line;
    try(line_init(&line), 0);
    while (fgets(buf, 512, inp) != NULL) {

    }
    return 1;
}

// Preprocesses the contents of 'inp', writes the result to the Text structure
int preprocess_file(FILE *inp, const char *path, Text *text) {
    // Split into lines
    Text lines;
    try(split(inp, &lines), 0);

    // For each line:
    text_destroy(&lines);
    return 1;
}

// Preprocesses pseudo.asm, followed by the input file
int preprocess(FILE *inp, const char *path, Text *text) {
    FILE *pseudo = fopen("pseudo.asm", "r");
    if (pseudo == NULL) {
        general_error(FILE_IO, __FILE__, "pseudo.asm");
        return 0;
    }
    try(preprocess_file(pseudo, "pseudo.asm", text), 0);

    try(preprocess_file(inp, path, text), 0)

    return 1;
}