#ifndef MIPS_ASSEMBLER_TEXT_H
#define MIPS_ASSEMBLER_TEXT_H
#include "strings.h"

/* === TYPES === */

typedef struct Line Line;
typedef struct Text Text;

// TODO: update this to use a String
struct Line {
    String *text;
    unsigned int number;

    void *next;
    void *prev;
};

struct Text {
    Line *head;
    Line *tail;
    unsigned int len;
    char *filename;
};

/* === METHODS === */

int line_init(Line *line);

int line_append(Line *line, char c);

int line_insert(Line *str, size_t index, const char *c);

void line_destroy(Line *line);

int text_init(Text *text, const char *fileName);

int text_add(Text *text, Line line);

Line * text_insert(Text *text, Line line, Line *before);

Line text_remove(Text *text, Line *line);

void text_destroy(const Text *text);

void text_debug(const Text *text);

#endif //MIPS_ASSEMBLER_TEXT_H