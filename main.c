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

int main(int argc, char *argv[]) {
    int nobjects, i;
    Object **objects, *result;
    FILE *file;
    char *text;
    Dictionary *dictionary;

    dictionary = dictionary_new();
    builtins_load(dictionary);

    if (argc == 1) {
        repl = true;
        setjmp(repl_jmp_buf);
        for (;;) {
            text = readline("> ");
            objects = parse(text, &nobjects);
            for (i = 0; i < nobjects; i++) {
                assert(objects[i]->type == LIST);
                result = eval(dictionary, objects[i]->u.list);
                object_print(result);
                // FIXME free on longjmp as well
                object_free(objects[i]);
                object_free(result);
                printf("\n");
            }
            free(objects);
        }
    } else if (argc == 2) {
        file = fopen(argv[1], "r");
        if (file == NULL)
            error_message("%s", strerror(errno));

        text = read_to_string(file);

        objects = parse(text, &nobjects);
        for (i = 0; i < nobjects; i++) {
            assert(objects[i]->type == LIST);
            object_print(eval(dictionary, objects[i]->u.list));
            printf("\n");
        }
    } else {
        error_message("Wrong number of arguments.");
    }

    return 0;
}
