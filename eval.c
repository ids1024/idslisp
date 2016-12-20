#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "eval.h"
#include "object.h"
#include "builtins.h"
#include "util.h"

Object *eval_arg(Dictionary *dictionary, Object *arg) {
    Object *value;

    if (arg->type == LIST) {
        // Replace list with what it evluates to
        value = eval(dictionary, arg);
    } else if (arg->type == SYMBOL) {
        value = dictionary_get(dictionary, arg->u.s);
        if (value == NULL)
            error_message("'%s' undefined.", arg->u.s);
        value->refcount++;
        garbage_collect(arg);
    } else {
        value = arg;
    }

    return value;
}

Object *eval(Dictionary *dictionary, Object *object) {
    char *command;
    ListNode *args;
    Object *function;
    ListNode *list;

    assert(object->type == LIST);
    list = object->u.list;
    object->u.list->refcount++;
    garbage_collect(object);

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
        return function->u.builtin(dictionary, args);
    } else if (function->type == SPECIAL)
        return function->u.builtin(dictionary, args);
    else
        error_message("'%s' not a function.", command);
}
