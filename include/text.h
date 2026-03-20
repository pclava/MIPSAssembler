#ifndef MIPS_ASSEMBLER_TEXT_H
#define MIPS_ASSEMBLER_TEXT_H
#include "strings.h"

/* === TYPES === */

typedef struct Line Line;
typedef struct Text Text;

struct Line {
    String *text;
    unsigned int number;

    Line *next;
    Line *prev;
};

struct Text {
    Line *head;
    Line *tail;
    size_t len;
    char *filename;
};

/* === METHODS === */

int line_init(Line *line);

int line_append(Line *line, char c);

int line_insert(Line *str, size_t index, const char *c);

int line_append_str(Line *str, const char *c);

char *line_get_str(const Line *str);

int line_cpy(Line *dest, const Line *src);

void line_destroy(Line *line);

int text_init(Text *text, const char *fileName);

int text_add(Text *text, Line line);

Line * text_insert(Text *text, Line line, Line *before);

Line text_remove(Text *text, Line *line);

void text_destroy(const Text *text);

void text_debug(const Text *text);

#endif //MIPS_ASSEMBLER_TEXT_H