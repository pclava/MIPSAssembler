#include "text.h"

#include <stdlib.h>
#include <string.h>

#include "utils.h"

/* Text

Used by the preprocessor to store the preprocessed input file
Implemented as a linked list of Line structures,
itself a dynamic array of characters.

Lines also contain information for error handling (filename and number)
*/

// Initializes and allocates memory
int line_init(Line *line, const char *fileName) {
    line->len = 0;
    line->cap = 64;
    line->number = -1;
    line->next = NULL;
    line->prev = NULL;

    char *text = malloc(line->cap);
    if (text == NULL) {
        general_error(MEM, __FILE__, NULL);
    }
    line->text = text;

    char *fname = malloc(strlen(fileName)+1);
    if (fname == NULL) {
        free(text);
        general_error(MEM, __FILE__, NULL);
        return 0;
    }

    strcpy(fname, fileName);
    line->filename = fname;
    return 1;
}

// Adds a character to the end of the array
int line_add_char(Line * line, const char c) {
    if (line->len >= line->cap) {
        line->cap *= 2;
        char *new = realloc(line->text, line->cap * sizeof(char));
        if (new == NULL) {
            general_error(MEM, __FILE__, NULL);
            return 0;
        }
        line->text = new;
    }
    line->text[line->len] = c;
    line->len++;
    return 1;
}

// Frees resources (text and filename)
void line_destroy(Line *line) {
    if (line->filename != NULL) {
        free(line->filename);
    }
    if (line->text != NULL) {
        free(line->text);
    }
}

// Initializes and allocates memory
int text_init(Text *text) {
    text->len = 0;
    text->head = NULL;
    text->tail = NULL;
    return 1;
}

// Adds a Line to the end of the array
int text_add(Text *text, const Line line) {
    Line *ptr = malloc(sizeof(Line));
    if (ptr == NULL) {
        raise_error(MEM, __FILE__, NULL);
        return 0;
    }
    *ptr = line;

    if (text->len == 0) {
        text->head = ptr;
        text->tail = ptr;
        ptr->next = NULL;
        ptr->prev = NULL;
    } else {
        ptr->prev = text->tail;
        text->tail->next = ptr;
        text->tail = ptr;
    }

    text->len++;
    return 1;
}

// Inserts a line in the linked list after `before`
// Returns pointer to inserted line
Line * text_insert(Text *text, Line line, Line *before) {
    Line *ptr = malloc(sizeof(Line));
    if (ptr == NULL) {
        raise_error(MEM, __FILE__, NULL);
        return 0;
    }
    *ptr = line;

    // before -> ptr -> old
    Line *old = before->next;
    before->next = ptr;
    ptr->next = old;
    ptr->prev = before;
    if (old != NULL) old->prev = ptr;
    text->len++;

    return ptr;
}

// Removes the line at that pointer and returns its value
Line text_remove(Text *text, Line *line) {
    // line.prev -> line -> line.next
    // line.prev -> line.next

    Line *prev = line->prev;
    Line *next = line->next;
    if (prev == NULL) {
        // Line is the head
        text->head = next;
    } else prev->next = next;
    if (next == NULL) {
        text->tail = prev;
    } else next->prev = prev;

    text->len--;
    const Line ret = *line;
    line_destroy(line);
    return ret;
}

// Frees resources (internal array and all Line resources)
void text_destroy(const Text *text) {
    Line *cur = text->head;
    while (cur != NULL) {
        Line *next = cur->next;
        line_destroy(cur);
        cur = next;
    }
}

void text_debug(const Text * text) {
    Line *cur = text->head;
    while (cur != NULL) {
        printf("%s\n", cur->text);
        cur = cur->next;
    }
}