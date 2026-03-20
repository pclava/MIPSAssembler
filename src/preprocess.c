#include "preprocess.h"
#include "utils.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include "pseudoinstructions.h"

FILE * open_file(const char *path) {
    FILE *inp = fopen(path, "r");
    if (inp == NULL) {
        general_error(FILE_IO, __FILE__, path);
        return NULL;
    }
    return inp;
}

// Checks if the line consists of just labels, in which case it collapses them to the next line
// Returns 0 on failure, 2 if the line must be removed, and 1 otherwise
int collapse_labels(const Line *line) {
    if (line == NULL) return 0;
    String *old = line->text;
    if (old == NULL) return 0;
    char *oldstr = old->str;
    if (old->len == 0) return 0;

    // If the last character is a colon, we assume the whole line is labels
    if (oldstr[old->len-1] == ':') {
        // Make sure there is a next line
        if (line->next == NULL) {
            raise_error(ARG_INV, oldstr, __FILE__);
            error_context("dangling label not supported");
            return 0;
        }

        // Insert line at start of next line
        try(string_append(line->text, ' '), 0);                 // space ensures padding between label and any subsequent text. may be removed when next line is sanitized
        try(string_insert(line->next->text, 0, oldstr), 0); // insert at start of next line

        // Mark line for removal
        return 2;
    }

    return 1;
}

// Identifies invocations of macros and replaces them
// Returns 0 on failure, 2 if the line must be removed, and 1 otherwise
int resolve_macros(Line *line, MacroTable *macro_table, Text *text) {
    if (line == NULL) return 0;
    String *old = line->text;
    if (old == NULL) return 0;
    char *oldstr = old->str;
    if (old->len == 0) return 1;

    char *buf = strdup(oldstr);
    long i;

    // Keep a new String, destroy and replace old String when finished
    String *new = malloc(sizeof(String));
    if (new == NULL) {
        raise_error(MEM, __FILE__, NULL);
        free(buf);
        return 0;
    }
    if (string_init(new) == 0) {
        free(new);
        free(buf);
        return 0;
    }

    // Tokenize by space and parentheses (skipping strings)
    // Note that sanitize guarantees that opening quotes are preceding by a space and closing quotes followed by one
    char *token = strtok(buf, " ()");
    unsigned char reading_string = 0;
    while (token != NULL) {
        size_t len = strlen(token);

        // skip label definitions and strings
        if (token[len-1] == ':') goto _add_and_continue;
        if (token[0] == '"') {
            reading_string ^= 1;
            goto _add_and_continue;
        }
        if (token[len-1] == '"' && token[len-2] != '\\') {
            reading_string = 0;
            goto _add_and_continue;
        }
        if (reading_string) goto _add_and_continue;

        // Check each token against macro_table
        Macro *macro;
        unsigned long index = mt_exists(macro_table, token);
        if (index != MACRO_TABLE_LENGTH) goto _insert;

        _add_and_continue:

        // Get character that was used as delimiter, so we can add it back
        // Note that sanitizer guarantees that spaces around parentheses are removed
        i = token-1-buf;    // index of previous delimiter
        if (i >= 0) {
            char c = oldstr[i];
            try(string_append(new, c), 0);
        }
        try(string_append_string(new, token), 0);
        token = strtok(NULL, " ()");
        if (token == NULL) {    // last delimiter
            char c = oldstr[i+len+1];   // also add last delimiter
            if (c != '\0') try(string_append(new, c), 0);
        }
        continue;

        _insert:
        macro = mt_get_at(macro_table, index);
        // Determine type
        if (macro->type == CONSTANT) {
            // Create a new Line after the current and copy everything into it. This is inefficient but its the easiest
            // way to ensure we go back over the macro to resolve any macros within it
            if (new->len != 0) try(string_append(new, ' '), 0);
            try(string_append_string(new, macro_get_constant(macro)), 0);

            Line to_insert;
            try(line_init(&to_insert), 0);
            to_insert.number = line->number;
            try(line_append_str(&to_insert, new->str), 0);
            try(text_insert(text, to_insert, line), 0);
            free(buf);
            return 2;   // delete old string
        }

        // Insert macro into list after current line (current line will then be removed)
        try(insert_macro(text, macro, line), 0);

        // Insert anything that was already in the String at the start of the next line
        if (new->len != 0) try(string_append(new, ' '), 0);
        try(line_insert(line->next, 0, new->str), 0);
        free(buf);
        return 2;

    }

    string_destroy(old);         // Delete old string
    line->text = new;           // Set new string
    free(buf);
    return 1;
}

// Returns length of macro (counting start and end)
size_t preproc_define_macro(const Line *line, MacroTable *macro_table) {
    Macro macro;
    if (define_macro(&macro, line) == NULL) return 0;
    try(mt_add(macro_table, macro), 0)
    return macro.definition_length+2;
}

// Returns success
int preproc_define_constant(const Line *line, MacroTable *macro_table) {
    Macro macro;
    if (define_constant(&macro, line) == 0) return 0;
    try(mt_add(macro_table, macro), 0);
    return 1;
}

// Puts in 'size' the length of the macro in lines (counting the start and end)
// If the function returns 2, the preprocessor removes the lines of the macro
int resolve_directives(Line *line, MacroTable *macro_table, size_t *end) {
    if (line == NULL) return 0;
    String *old = line->text;
    if (old == NULL) return 0;
    char *oldstr = old->str;
    if (old->len == 0) return 0;

    if (oldstr[0] != '.') return 1; // no directive

    // Copy into a buffer for tokenization
    char buf[old->len+1];
    strcpy(buf, oldstr);

    // The preprocessor recognizes the directives: .eqv, .define, .macro
    // If it doesn't recognize a directive, it assumes the assembler will handle it
    char *directive = tokenize(buf, ' ');

    // .eqv and .define do the same thing, which defines a symbolic constant
    if (strcmp(directive, ".eqv") == 0 || strcmp(directive, ".define") == 0) {
        if (preproc_define_constant(line, macro_table) == 0) return 0;
        *end = 1;
        return 2;
    }

    // .macro defines a macro, or a reusable block of code. must be closed with .end_macro
    if (strcmp(directive, ".macro") == 0) {
        // MACRO
        size_t size = preproc_define_macro(line, macro_table);   // get number of lines to remove
        if (size == 0) return 0;                                 // failed
        *end = size;
        return 2;                                                // flag for removal
    }

    // If .end_macro is found here, it doesn't have a corresponding .macro
    if (strcmp(directive, ".end_macro") == 0) {
        raise_error(TOKEN_ERR, directive, __FILE__);
        error_context("found .end_macro without preceding .macro");
        return 0;
    }

    return 1;
}

// Normalizes whitespace, removes commas, removes comments, ensures spaces between strings, clears whitespace between ()
// Returns 1 on success, or 2 if the line in question should be removed from the list
int sanitize(Line *line) {
    if (line == NULL) return 0;
    String *old = line->text;
    if (old == NULL) return 0;
    char *oldstr = old->str;
    if (old->len == 0) return 1;

    // Create a new String and write the sanitized string there
    String *new = malloc(sizeof(String));
    if (new == NULL) {
        raise_error(MEM, __FILE__, NULL);
        return 0;
    }
    if (string_init(new) == 0) {
        free(new);
        return 0;
    }

    // Fill new string
    unsigned char prev_wsp = 1;  // boolean: is the previous character a whitespace? initially, we say yes to remove initial whitespace
    unsigned char reading_string = 0;   // boolean: are the current characters inside a user string? if so, don't skip whitespace
    for (size_t i = 0; i < old->len; i++) {
        // Skip extra whitespace (treat comma as whitespace)
        if ((oldstr[i] == ',' || isspace(oldstr[i])) && !reading_string) {
            if (prev_wsp || oldstr[i+1] == COMMENT) continue; // skip preceding whitespace and whitespace before a comment

            // For consistency, replace all whitespace with 0x20 ' '
            string_append(new, ' ');
            prev_wsp = 1;
            continue;
        }
        // Break if comment
        if (oldstr[i] == COMMENT) break;

        // Otherwise, we should add the character
        if (oldstr[i] == '"') {
            if (!reading_string) {  // put space before opening quote
                if (!prev_wsp) string_append(new, ' ');
                string_append(new, oldstr[i]);
                prev_wsp = 0;
            }
            else {                // put space after closing quote
                string_append(new, oldstr[i]);
                string_append(new, ' ');
                prev_wsp = 1;
            }
            // Toggle reading_string if the character is double-quotes but not if being used as the escape sequence \"
            if (i == 0 || oldstr[i-1] != '\\') reading_string ^= 1;
            continue;
        }

        if ((oldstr[i] == '(' || oldstr[i] == ')') && !reading_string) {
            if (prev_wsp) { // remove previous whitespace
                new->len--;
                new->str[new->len] = '\0';
            }
            prev_wsp = 1;   // to omit whitespace after parenthesis
        }
        else prev_wsp = 0;
        string_append(new, oldstr[i]);
    }

    // Loop backwards and remove final whitespace
    if (new->len > 0) {
        size_t i = new->len-1;
        while (i > 0 && isspace(new->str[i])) {
            new->str[i] = '\0';
            new->len--;
            i--;
        }
    }
    // If the new string is empty, delete the original line and return
    else if (new->len == 0) {
        return 2;
    }

    // New string is not empty, add to list
    string_destroy(old);         // Delete old string
    line->text = new;           // Set new string
    return 1;
}

// Splits a file into lines and writes those to the Text structure
int split(FILE *inp, Text *lines) {
    char buf[512];
    Line line;
    try(line_init(&line), 0);
    int line_no = 1;
    while (fgets(buf, sizeof(buf), inp) != NULL) {
        line_append_str(&line, buf);
        if (strchr(buf, '\n')) {
            // Finished reading line, add to Text and reset line
            line.number = line_no;
            try(text_add(lines, line), 0);
            try(line_init(&line), 0);
            line_no++;
        } else if (feof(inp)) {
            // Finished reading the last line
            line.number = line_no;
            try(text_add(lines, line), 0);
            break;
        }
        // Did not finish reading line or reach end of file, read more from buf and keep appending
    }
    return 1;
}

// Preprocesses the contents of 'inp', writes the result to the Text structure
int preprocess_file(FILE *inp, Text *text, MacroTable *macro_table) {
    try(split(inp, text), 0);                      // Split into lines

    // Sanitize
    Line *cur = text->head;
    while (cur != NULL) {
        ERROR_HANDLER.line = cur;
        Line *next = cur->next;
        int success = sanitize(cur);
        try(success, 0);
        if (success == 2) {
            text_remove(text, cur);
        }
        cur = next;
    }

    // Process each line separately
    cur = text->head;
    while (cur != NULL) {
        ERROR_HANDLER.line = cur;
        Line *next = cur->next;     // remember where

        // Resolve directives
        size_t end;
        int success = resolve_directives(cur, macro_table, &end);
        try(success, 0);
        if (success == 2) {
            // Remove every line from cur to end
            Line *l = cur;
            for (size_t i = 0; i < end; i++) {
                Line *n = l->next;
                text_remove(text, l);
                l = n;
            }
            next = l;
            goto _nextline;
        }

        // Collapse lonely labels
        success = collapse_labels(cur);
        try(success, 0);
        if (success == 2) {
            text_remove(text, cur);
            goto _nextline;
        }

        // Resolve macro usages
        success = resolve_macros(cur, macro_table, text);
        try(success, 0);
        if (success == 2) {
            next = cur->next;       // resolve_macros inserts new lines, which affects the 'next'
            text_remove(text, cur);
        }

        _nextline:
        cur = next;
    }

    // text_debug(text);
    // mt_debug(macro_table);

    return 1;
}

// Preprocesses pseudo.asm, followed by the input file
int preprocess(FILE *inp, Text *text) {
    FILE *pseudo = fopen("pseudo.asm", "r");
    if (pseudo == NULL) {
        general_error(FILE_IO, __FILE__, "pseudo.asm");
        return 0;
    }

    MacroTable *macro_table = malloc(sizeof(MacroTable));
    if (macro_table == NULL) {
        raise_error(MEM, NULL, __FILE__);
        return 0;
    }
    try(mt_init(macro_table), 0);

    try(preprocess_file(pseudo, text, macro_table), 0);
    try(preprocess_file(inp, text, macro_table), 0)

    mt_destroy(macro_table);

    return 1;
}