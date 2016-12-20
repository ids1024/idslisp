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
    } else {
        value = arg;
        value->refcount++;
    }

    return value;
}

Object *eval(Dictionary *dictionary, Object *object) {
    char *command;
    ListNode *args;
    Object *function, *value;
    ListNode *list, *tmpnodes=NULL, *last_tmpnode, *tmpnode;

    assert(object->type == LIST);
    list = object->u.list;

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
            tmpnode = new_node(NULL, eval_arg(dictionary, node->value));
            if (tmpnodes == NULL)
                tmpnodes = tmpnode;
            else
                last_tmpnode->next = tmpnode;
            last_tmpnode = tmpnode;
        }
        value = function->u.builtin(dictionary, tmpnodes);
        garbage_collect_list(tmpnodes);
    } else if (function->type == SPECIAL)
        value = function->u.builtin(dictionary, args);
    else
        error_message("'%s' not a function.", command);

    return value;
}
