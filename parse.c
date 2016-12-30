#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "parse.h"
#include "object.h"
#include "tokenize.h"
#include "util.h"
#include "sequence.h"


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
        item = new_cons(_parse_one(tokens, ntoks, i), &NIL_CONST);
        item = new_cons(new_symbol("quote"), item);
    } else if (strcmp(tokens[*i], "(") == 0) {
        (*i)++;
        item = _parse_iter(tokens, ntoks, i);
        if (strcmp(tokens[*i], ")") != 0)
            error_message("'(' with no matching ')'");
    } else if (strcmp(tokens[*i], "[") == 0) {
        (*i)++;
        item = _parse_iter(tokens, ntoks, i);
        if (strcmp(tokens[*i], "]") != 0)
            error_message("'[' with no matching ']'");
        item = new_cons(new_symbol("vector"), item);
    } else if (strcmp(tokens[*i], ")") == 0 || strcmp(tokens[*i], "]") == 0) {
        item = NULL;
    } else {
        item = _str_to_num_object(tokens[*i]);
        if (item == NULL)
            item = new_symbol(tokens[*i]);
    }

    return item;
}

Object *_parse_iter(char **tokens, int ntoks, int *i) {
    Object *item, *car, *cdr,  *list=&NIL_CONST, *prev_node;

    for (; *i<ntoks; (*i)++) {
        item = _parse_one(tokens, ntoks, i);

        if (item == NULL) {
            if (seq_len(list) == 3 &&
                seq_nth(list, 1)->type == SYMBOL &&
                strcmp(seq_nth(list, 1)->u.s, ".") == 0) {
                // Cons (eg. (1 . 2))
                car = ref(seq_nth(list, 0));
                cdr = ref(seq_nth(list, 2));
                garbage_collect(list);
                return new_cons(car, cdr);
            }
            return list;
        }

        // Append to linked list
        append_node(&list, &prev_node, item);
    }

    error_message("'(' with no matching ')'");
}

Object **parse(char *code, int *nobjects) {
    int ntoks, i;
    char **tokens;
    Object *object, **objects = NULL;
    
    *nobjects = 0;
    tokens = tokenize(code, &ntoks);

    for (i=0; i<ntoks; i++) {
        object = _parse_one(tokens, ntoks, &i);

        if (object == NULL)
            error_message("')' with no matching '('");

        array_append(objects, *nobjects, object);
    }

    // Free the tokens array
    for (i=0; i<ntoks; i++)
        free(tokens[i]);
    free(tokens);

    return objects;
}
