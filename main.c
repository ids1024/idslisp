/** @file */ 

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "object.h"
#include "parse.h"
#include "eval.h"
#include "util.h"
#include "dictionary.h"
#include "builtins.h"

/**
 * @brief Create string with contents of FILE
 */
char *read_to_string(FILE *file) {
    // FIXME: performance
    char *text=NULL, next_char;
    int len=0;

    while ((next_char = fgetc(file)) != EOF)
	array_append(text, len, next_char);
    array_append(text, len, '\0');

    return text;
}

/**
 * @brief Open file and read to string
 * @param filename Path to file
 */
char *read_file(char *filename) {
    char *text;
    FILE *file;
    
    file = fopen(filename, "r");
    if (file == NULL)
        error_message("%s", strerror(errno));
    text = read_to_string(file);
    fclose(file);
    return text;
}

/**
 * @brief Parse and evaluate lisp code
 * @param dictionary Dictionary to evaluate code in
 * @param text The lisp code
 * @param print If true, print return value(s)
 */
void parse_and_eval(Dictionary *dictionary, char *text, bool print) {
    Object **objects, *result;
    int nobjects, i;

    objects = parse(text, &nobjects);
    for (i = 0; i < nobjects; i++) {
        if (setjmp(error_jmp_buf) == 0) {
            result = eval(dictionary, objects[i]);
            if (print) {
                object_print(result);
                printf("\n");
            }
            garbage_collect(result);
        }
        garbage_collect(objects[i]);
    }
    free(objects);
}

int main(int argc, char *argv[]) {
    char *text;
    Dictionary *dictionary;
    int status = 0;

    dictionary = dictionary_new(NULL);
    builtins_load(dictionary);

    if (argc == 1) {
        while ((text = readline("> ")) != NULL) {
            add_history(text);
            parse_and_eval(dictionary, text, true);
            free(text);
        }
	printf("\n");
    } else if (argc == 2) {
        if (setjmp(error_jmp_buf) == 0) {
            text = read_file(argv[1]);
            parse_and_eval(dictionary, text, false);
            free(text);
        } else
            status = 1;
    } else {
        error_message("Wrong number of arguments.");
    }

    dictionary_free(dictionary);

    return status;
}
