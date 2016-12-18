#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "object.h"
#include "tokenize.h"


Object *_str_to_num_object(char *text) {
    char *endptr;
    long int inum;
    double fnum;

    inum = strtol(text, &endptr, 10);
    if (*(endptr+1) == '\0')
        return new_int(inum);

    fnum = strtod(text, &endptr);
    if (*(endptr+1) == '\0')
        return new_double(fnum);

    return NULL;
}

Object *_parse_iter(char **tokens, int ntoks, int *i) {
    int j;
    Object *item;
    ListNode *list=NULL, *prev_node=NULL, *node;

    for (; *i<ntoks; (*i)++) {
        if (strcmp(tokens[*i], "\"") == 0) {
            assert((ntoks >= *i+3) && (strcmp(tokens[*i+2], "\"") == 0));
            item = new_string(tokens[*i+1]);
            (*i) += 2;
        } else if (strcmp(tokens[*i], "(") == 0) {
            (*i)++;
            item = _parse_iter(tokens, ntoks, i);
        } else if (strcmp(tokens[*i], ")") == 0) {
            return new_list(list);
        } else {
            item = _str_to_num_object(tokens[*i]);
            if (item == NULL) {
                printf("Error: token '%s' not recognized.", tokens[*i]);
                exit(1);
            }
        }

        // Append to linked list
        node = new_node(NULL, item);
        if (list == NULL)
            list = node;
        if (prev_node != NULL)
            prev_node->next = node;
        prev_node = node;
    }

    printf("Error: missing ')'\n");
    exit(1);
}

Object **parse(char *code, int *nobjects) {
    int ntoks, i;
    char **tokens;
    Object *object, **objects = NULL;
    
    *nobjects = 0;
    tokens = tokenize(code, &ntoks);

    for (i=0; i<ntoks; i++) {
        if (strcmp(tokens[i], "(") == 0) {
            i++;
            object = _parse_iter(tokens, ntoks, &i);

            (*nobjects)++;
            objects = realloc(objects, *nobjects * sizeof(Object*));
            objects[*nobjects-1] = object;
        } else {
            printf("Error: '%s' not allowed here.", tokens[i]);
            exit(1);
        }
    }

    // Free the tokens array
    for (i=0; i<ntoks; i++)
        free(tokens[i]);
    free(tokens);

    return objects;
}
