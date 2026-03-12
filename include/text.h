#ifndef MIPS_ASSEMBLER_TEXT_H
#define MIPS_ASSEMBLER_TEXT_H

/* === TYPES === */

typedef struct Line Line;
typedef struct Text Text;

// TODO: update this to use a String
struct Line {
    unsigned int len;
    unsigned int cap;

    char *text;
    unsigned int number;
    char *filename;

    void *next;
    void *prev;
};

struct Text {
    Line *head;
    Line *tail;
    unsigned int len;
};

/* === METHODS === */

int line_init(Line *line, const char *fileName);

int line_add_char(Line *line, char c);

void line_destroy(Line *line);

int text_init(Text *text);

int text_add(Text *text, Line line);

Line * text_insert(Text *text, Line line, Line *before);

Line text_remove(Text *text, Line *line);

void text_destroy(const Text *text);

void text_debug(const Text *text);

#endif //MIPS_ASSEMBLER_TEXT_H