#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "object.h"
#include "parse.h"
#include "sequence.h"
#include "tokenize.h"
#include "util.h"

Object *_parse_iter(char **tokens, int ntoks, int *i);

Object *_str_to_num_object(char *text) {
    char *endptr;

    long int inum = strtol(text, &endptr, 10);
    if (*endptr == '\0')
        return new_int(inum);

    double fnum = strtod(text, &endptr);
    if (*endptr == '\0')
        return new_double(fnum);

    return NULL;
}

Object *_parse_one(char **tokens, int ntoks, int *i) {
    Object *item;

    if (strcmp(tokens[*i], "\"") == 0) {
        assert((ntoks >= *i + 3) && (strcmp(tokens[*i + 2], "\"") == 0));
        item = new_string(tokens[*i + 1]);
        (*i) += 2;
    } else if (strncmp(tokens[*i], "\\", 1) == 0) {
        if (strlen(tokens[*i]) != 2)
            error_message("Invalid character.");
        item = new_character(tokens[*i][1]);
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
    Object *list = &NIL_CONST, *prev_node;

    TRY_START(1, list);

    for (; *i < ntoks; (*i)++) {
        Object *item = _parse_one(tokens, ntoks, i);

        if (item == NULL) {
            if (seq_len(list) == 3 && seq_nth(list, 1)->type == SYMBOL &&
                strcmp(seq_nth(list, 1)->u.s, ".") == 0) {
                // Cons (eg. (1 . 2))
                Object *car = ref(seq_nth(list, 0));
                Object *cdr = ref(seq_nth(list, 2));
                garbage_collect(list);
                TRY_END();
                return new_cons(car, cdr);
            }
            TRY_END();
            return list;
        }

        // Append to linked list
        append_node(&list, &prev_node, item);
    }

    error_message("'(' with no matching ')'");
}

/**
 * @brief Parses source code to objects
 * @param code Source code to parse
 * @param nobjects Number of objects read will be written here
 */
Object **parse(char *code, int *nobjects) {
    Object **objects = NULL;
    int ntoks;

    *nobjects = 0;
    char **tokens = tokenize(code, &ntoks);

    jmp_buf tmp_error_jmp_buf;
    memcpy(tmp_error_jmp_buf, error_jmp_buf, sizeof(jmp_buf));
    if (setjmp(error_jmp_buf) != 0) {
        for (int i = 0; i < ntoks; i++)
            free(tokens[i]);
        free(tokens);
	for (int i = 0; i < *nobjects; i++)
            garbage_collect(objects[i]);
	free(objects);
        TRY_END();
        longjmp(error_jmp_buf, 1);
    }

    for (int i = 0; i < ntoks; i++) {
        Object *object = _parse_one(tokens, ntoks, &i);

        if (object == NULL)
            error_message("')' with no matching '('");

        array_append(objects, *nobjects, object);
    }

    // Free the tokens array
    for (int i = 0; i < ntoks; i++)
        free(tokens[i]);
    free(tokens);

    TRY_END();
    return objects;
}
