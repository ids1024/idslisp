#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "parse.h"
#include "object.h"
#include "tokenize.h"
#include "util.h"


Object *_parse_iter(char **tokens, int ntoks, int *i);


Object *_str_to_num_object(char *text) {
    char *endptr;
    long int inum;
    double fnum;

    inum = strtol(text, &endptr, 10);
    if (*endptr == '\0')
        return new_int(inum);

    fnum = strtod(text, &endptr);
    if (*endptr == '\0')
        return new_double(fnum);

    return NULL;
}

Object *_parse_one(char **tokens, int ntoks, int *i) {
    Object *item;

    if (strcmp(tokens[*i], "\"") == 0) {
        assert((ntoks >= *i+3) && (strcmp(tokens[*i+2], "\"") == 0));
        item = new_string(tokens[*i+1]);
        (*i) += 2;
    } else if (strcmp(tokens[*i], "'") == 0) {
        (*i)++;
        item = new_list(new_node(NULL, _parse_one(tokens, ntoks, i)));
        item->u.list = new_node(item->u.list, new_symbol("quote"));
    } else if (strcmp(tokens[*i], "(") == 0) {
        (*i)++;
        item = _parse_iter(tokens, ntoks, i);
    } else if (strcmp(tokens[*i], ")") == 0) {
        item = NULL;
    } else {
        item = _str_to_num_object(tokens[*i]);
        if (item == NULL)
            item = new_symbol(tokens[*i]);
    }

    return item;
}

Object *_parse_iter(char **tokens, int ntoks, int *i) {
    Object *item;
    ListNode *list=NULL, *prev_node;

    for (; *i<ntoks; (*i)++) {
        item = _parse_one(tokens, ntoks, i);

        if (item == NULL)
            return new_list(list);

        // Append to linked list
        append_node(&list, &prev_node, item);
    }

    error_message("missing ')'");
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
            error_message("'%s' not allowed here.", tokens[i]);
        }
    }

    // Free the tokens array
    for (i=0; i<ntoks; i++)
        free(tokens[i]);
    free(tokens);

    return objects;
}
