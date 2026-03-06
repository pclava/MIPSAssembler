#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "preprocess.h"
#include "linker.h"

/*
 USAGE:
 $ mips_assembler <options> <source files>
 Note the files are linked *in the order they are listed*
 The options include:
 -o <file>      writes the executable to the given file (if not provided, outputs to a.out). ignored if -c flag is provided
 -c             stops after assembler, outputs unlinked object files for each source
 -e.            begin execution at the first instruction
 -e <symbol>    begin execution at the given global symbol (if not provided, linker links __start and begins execution there)
 */

int main(int argc, char *argv[]) {
    int performLinking = 1;
    int clean = 1;
    if (argc < 2) {
        fprintf(stderr, "error in %s: invalid arguments\n", __FILE__);
        return 1;
    }

    char *entry = "__start"; // symbol that execution should begin at; if null, begins at TEXT_START (0x00400000)
    char *out_path = "a.out";
    char **in_files = malloc((argc-1) * sizeof(char *));
    if (in_files == NULL) {
        raise_error(MEM, NULL, __FILE__);
        free(in_files);
        return 1;
    }
    int file_count = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            switch (argv[i][1]) {
                case 'c':
                    performLinking = 0;
                    out_path = NULL;
                    break;
                case 'e':
                    performLinking = 0;
                    if (argv[i][2] == '.') {
                        entry = NULL;
                    } else {
                        i++;
                        entry = argv[i];
                    }
                    break;
                case 'o':
                    i++;
                    out_path = argv[i];
                    break;
                default:
                    fprintf(stderr, "error in %s: unrecognized option %c\n", __FILE__, argv[i][1]);
                    free(in_files);
                    return 1;
            }
        }
        // input file
        else {
            in_files[file_count++] = argv[i];
        }
    }

    char *object_files[file_count+1];

    // Assemble each source file
    for (int i = 0; i < file_count; i++) {
        char *inp_path = in_files[i];

        // Strip suffix from input path and add .o
        char *object_path = malloc(strlen(inp_path)+3);
        size_t j;
        for (j = 0; j < strlen(inp_path); j++) {
            if (inp_path[j] == '.' || inp_path[j] == '0') break;
            object_path[j] = inp_path[j];
        }
        object_path[j++] = '.';
        object_path[j++] = 'o';
        object_path[j] = '\0';
        object_files[i] = object_path;

        FILE *inp_file = open_file(inp_path);
        if (inp_file == NULL) {
            free(in_files);
            return 1;
        }

        Text text;
        text_init(&text);

        if (preprocess(inp_file, inp_path, &text) == 0) {
            fprintf(stderr, "Error in %s: could not preprocess file \"%s\"\n", __FILE__, inp_path);
            text_destroy(&text);
            for (int k = 0; k <= i-2; k++) {
                free(object_files[k]);
            }
            free(in_files);
            return 2;
        }

        // text_debug(&text);

        if (assemble(&text, object_path) == 0) {
            fprintf(stderr, "Error in %s: could not assemble file \"%s\"\n", __FILE__, inp_path);
            text_destroy(&text);
            for (int k = 0; k <= i-2; k++) {
                free(object_files[k]);
            }
            free(in_files);
            return 3;
        }

        // debug_binary(object_path);
        text_destroy(&text);
    }

    if (performLinking) {
        if (link(out_path, object_files, file_count, entry) == 0) {
            fprintf(stderr, "Error in %s: could not link files\n", __FILE__);
            for (int i = 0; i < file_count; i++) {
                free(object_files[i]);
            }
            free(in_files);
            return 4;
        }
    }

    for (int i = 0; i < file_count; i++) {
        if (performLinking && clean) remove(object_files[i]);
        free(object_files[i]);
    }
    free(in_files);

    return 0;
}