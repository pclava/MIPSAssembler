#include "text.h"

#include <signal.h>
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
int line_init(Line *line) {
    line->number = -1;
    line->next = NULL;
    line->prev = NULL;

    String *str = malloc(sizeof(String));
    if (str == NULL) {
        raise_error(MEM, __FILE__, NULL);
        return 0;
    }
    if (string_init(str) == 0) {
        free(str);
        return 0;
    }
    line->text = str;

    return 1;
}

// Adds a character to the end of the array
int line_append(Line *line, const char c) {
    return string_append(line->text, c);
}

int line_insert(Line *str, size_t index, const char *c) {
    return string_insert(str->text, index, c);
}

// Frees resources (text and filename)
void line_destroy(Line *line) {
    if (line->text != NULL) {
        string_destroy(line->text);
    }
}

// Initializes and allocates memory
int text_init(Text *text, const char *fileName) {
    text->len = 0;
    text->head = NULL;
    text->tail = NULL;
    text->filename = NULL;
    if (fileName != NULL) {
        text->filename = strdup(fileName);
        if (text->filename == NULL) {
            raise_error(MEM, __FILE__, NULL);
            return 0;
        }
    }

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
    if (text == NULL) return;
    Line *cur = text->head;
    while (cur != NULL) {
        Line *next = cur->next;
        line_destroy(cur);
        cur = next;
    }
    free(text->filename);
}

void text_debug(const Text * text) {
    Line *cur = text->head;
    while (cur != NULL) {
        printf("%d: %s", cur->number, cur->text->str);
        cur = cur->next;
    }
}