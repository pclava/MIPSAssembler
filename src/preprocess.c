#include "preprocess.h"
#include "utils.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

/* Preprocessor

Reads through the input file:
- Removes comments
- Removes unnecessary whitespace
- Moves labels inline
*/

FILE * open_file(const char *path) {
    FILE *inp = fopen(path, "r");
    if (inp == NULL) {
        general_error(FILE_IO, __FILE__, path);
        return NULL;
    }
    return inp;
}

// Preprocesses the contents of 'inp', writes the result to the Text structure
int preprocess_file(FILE *inp, const char *path, Text *text) {
    int c;
    int prev = '\0';

    Line line;
    line_init(&line, path);
    unsigned int line_number = 1; // Increment on every \n
    line.number = line_number;

    int readingString = 0;
    while ((c = fgetc(inp)) != EOF) {

        // End of line, add to list
        if (c == '\n') {
            line_number++;
            if (prev == '\0') {
                line.number = line_number;
                continue;
            } // Skip if empty line
            if (isspace(prev)) line.text[line.len-1] = '\0'; // overwrite trailing space
            else line_add_char(&line, '\0'); // otherwise append null terminator
            text_add(text, line);
            line_init(&line, path); // Reset line
            line.number = line_number;
            prev = '\0'; // Reset prev
            continue;
        }

        if (!readingString) { // Character is not in a string
            if (c == '\"' && prev != '\\') {
                readingString = 1;
            }

            else if (c == ',') {
                if (isspace(prev) || prev == '\0') continue;
                line_add_char(&line, ' ');
                prev = ' ';
                continue;
            }

            // Skip whitespace if previous character was also whitespace
            else if (isspace(c) && (isspace(prev) || prev == '\0')) {
                continue; // without updating prev
            }
            // Replace tab with space
            else if (c == 9) c = ' ';

            // If character is a comment, stop reading and add the line as is
            else if (c == '#') {
                if (prev != '\0') { // Add the line
                    if (isspace(prev)) line.text[line.len-1] = '\0'; // overwrite trailing space
                    else line_add_char(&line, '\0'); // otherwise append null terminator
                    text_add(text, line);
                    line_init(&line, path); // Reset line
                    line.number = line_number;
                    prev = '\0'; // Reset prev
                }

                // Read until next line
                do {
                    c = fgetc(inp);
                } while (c != EOF && c != '\n');
                if (c == EOF) break;
                line_number++;
                line.number++;

                continue;
            }

            // Labels should be moved to the next line with text
            else if (c == ':') {
                // skip over whitespace and comments until an instruction is found
                line_add_char(&line, (char) c);
                line_add_char(&line, ' ');
                prev = ' ';
                c = fgetc(inp);
                while (isspace(c) || c == '#') {
                    if (c == '\n') {
                        line_number++; // increment both number counter and line's number
                        line.number++; // because line's number was the start of the label
                    }
                    else if (c == '#') {
                        // Loop until end of line
                        while (c != '\n' && c != EOF) {
                            c = fgetc(inp);
                        }
                        if (c == EOF) break;
                        line.number++;
                        line_number++;
                    }
                    else if (c == EOF) break;
                    c = fgetc(inp);
                }
                if (c == EOF) break;
                line_add_char(&line, (char) c);

                continue;
            }

            line_add_char(&line, (char) c);
        }

        else { // Character is part of a string
            // Check for end quote
            if (c == '\"' && prev != '\\') {
                readingString = 0;
            }

            // Add character
            line_add_char(&line, (char) c);
        }

        prev = c;
    }

    // Add last line
    if (prev != '\0') {
        line_add_char(&line, '\0');
        text_add(text, line);
    }

    fclose(inp);
    return 1;
}

// Preprocesses pseudo.asm, followed by the input file
int preprocess(FILE *inp, const char *path, Text *text) {
    FILE *pseudo = fopen("pseudo.asm", "r");
    if (preprocess_file(pseudo, "pseudo.asm", text) == 0) return 0;

    if (preprocess_file(inp, path, text) == 0) return 0;

    return 1;
}