#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#include <readline/readline.h>

#include "object.h"
#include "parse.h"
#include "eval.h"
#include "util.h"
#include "dictionary.h"
#include "builtins.h"

char *read_to_string(FILE *file) {
    // FIXME: performance
    char *text, next_char;
    int len=0;

    text = malloc(1);
    while ((next_char = fgetc(file)) != EOF) {
        len++;
        text = realloc(text, len+1);
        text[len-1] = next_char;
    }
    text[len] = '\0';

    return text;
}

void parse_and_eval(Dictionary *dictionary, char *text, bool print) {
    Object **objects, *result;
    int nobjects, i;

    objects = parse(text, &nobjects);
    for (i = 0; i < nobjects; i++) {
        assert(objects[i]->type == LIST);
        result = eval(dictionary, objects[i]);
        if (print) {
            object_print(result);
            printf("\n");
        }
        // FIXME free on longjmp as well
        garbage_collect(objects[i]);
        garbage_collect(result);
    }
    free(objects);
}

int main(int argc, char *argv[]) {
    FILE *file;
    char *text;
    Dictionary *dictionary;
    int status = 0;

    dictionary = dictionary_new(NULL);
    builtins_load(dictionary);

    if (argc == 1) {
        setjmp(error_jmp_buf);
        while ((text = readline("> ")) != NULL) {
            parse_and_eval(dictionary, text, true);
            free(text);
        }
	printf("\n");
    } else if (argc == 2) {
        if (setjmp(error_jmp_buf) == 0) {
            file = fopen(argv[1], "r");
            if (file == NULL)
                error_message("%s", strerror(errno));
            text = read_to_string(file);
            fclose(file);

            parse_and_eval(dictionary, text, false);
        }else
            status = 1;
        free(text);
    } else {
        error_message("Wrong number of arguments.");
    }

    dictionary_free(dictionary);

    return status;
}
