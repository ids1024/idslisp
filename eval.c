#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eval.h"
#include "object.h"
#include "builtins.h"
#include "util.h"

Object *eval_arg(Dictionary *dictionary, Object *arg) {
    Object *value;

    if (arg->type == LIST) {
        // Replace list with what it evluates to
        value = eval(dictionary, arg->u.list);
        //object_free(arg) XXX;
    } else if (arg->type == SYMBOL) {
        value = dictionary_get(dictionary, arg->u.s);
        if (value == NULL)
            error_message("'%s' undefined.", arg->u.s);
    } else {
        value = arg;
    }

    return value;
}

Object *eval(Dictionary *dictionary, ListNode *list) {
    char *command;
    ListNode *args;
    Object *function;

    if (list == NULL)
        error_message("Cannot evaluate empty list.");
    else if (list->value->type != SYMBOL)
        error_message("Cannot evaluate non-symbol.");

    command = list->value->u.s;
    args = list->next;
    function = dictionary_get(dictionary, command);

    // Execute command
    if (function == NULL)
        error_message("'%s' undefined.", command);
    else if (function->type == BUILTIN) {
        // Recursively evaluate arguments
        for (ListNode *node=args; node!=NULL; node=node->next) {
            node->value = eval_arg(dictionary, node->value);
        }
        function->u.builtin(dictionary, args);
    } else if (function->type == SPECIAL)
        function->u.builtin(dictionary, args);
    else
        error_message("'%s' not a function.", command);
}
