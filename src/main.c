#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "assembler.h"
#include "preprocess.h"
#include "utils.h"

/*
 Main function for assembler
 Usage:
 MIPSAssembler <files>

 Outputs object files with the same name as their source and with the .o extension
 */

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "error in %s: invalid arguments\n", __FILE__);
        return 1;
    }

    char **in_files = malloc((argc-1) * sizeof(char *));
    if (in_files == NULL) {
        raise_error(MEM, NULL, __FILE__);
        return 1;
    }
    int file_count = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            fprintf(stderr, "error in %s: skipping unrecognized option \"%s\"\n", __FILE__, argv[i]);
            continue;
        }
        in_files[file_count++] = argv[i];
    }

    char *object_files[file_count];

    // Assemble each source file
    for (int i = 0; i < file_count; i++) {
        char *inp_path = in_files[i];

        // Strip suffix from input path and add .o
        char *object_path = malloc(strlen(inp_path)+3);
        char *p = strrchr(inp_path, '.');
        size_t j;
        if (p == NULL) j = strlen(inp_path);
        else {
            j = (size_t) (p - inp_path);
        }
        strncpy(object_path, inp_path, j);
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

    for (int i = 0; i < file_count; i++) {
        free(object_files[i]);
    }
    free(in_files);

    return 0;
}