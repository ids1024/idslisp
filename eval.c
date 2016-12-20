#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "eval.h"
#include "object.h"
#include "builtins.h"
#include "util.h"
#include "dictionary.h"

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

Object *call_user_function(Dictionary *dictionary, Object *function, ListNode *args) {
    Dictionary *local_dictionary;
    ListNode *arg_defs, *arg, *node;
    Object *value;

    assert(function->type == FUNCTION);
    assert(function->u.list->value->type == LIST);

    // Set arguments as local variables
    local_dictionary = dictionary_new(dictionary);
    node = function->u.list->value->u.list;
    arg = args;
    while (node != NULL) {
        assert(node->value->type == SYMBOL);
        if (arg == NULL)
            error_message("Wrong number of arguments to function.");

        arg->value->refcount++;
        dictionary_insert(local_dictionary, node->value->u.s, arg->value);
        arg = arg->next;
        node=node->next;
    }

    // Execute code; last value will be return value
    value = new_nil();
    node = node=function->u.list->next;
    while (node != NULL) {
        garbage_collect(value);
        value = eval_arg(local_dictionary, node->value);
        node = node->next;
    }

    dictionary_free(local_dictionary);

    return value;
}

Object *call_function(Dictionary *dictionary, Object *function, ListNode *args) {
    Object *value;
    ListNode *tmpnodes=NULL, *last_tmpnode, *tmpnode;
 
    if (function->type == BUILTIN || function->type == FUNCTION) {
        // Recursively evaluate arguments
        for (ListNode *node=args; node!=NULL; node=node->next) {
            tmpnode = new_node(NULL, eval_arg(dictionary, node->value));
            if (tmpnodes == NULL)
                tmpnodes = tmpnode;
            else
                last_tmpnode->next = tmpnode;
            last_tmpnode = tmpnode;
        }
        if (function->type == BUILTIN)
            value = function->u.builtin(dictionary, tmpnodes);
        else if (function->type == FUNCTION) {
            value = call_user_function(dictionary, function, tmpnodes);
        }
        garbage_collect_list(tmpnodes);
    } else if (function->type == SPECIAL)
        value = function->u.builtin(dictionary, args);

    return value;
}

Object *eval(Dictionary *dictionary, Object *object) {
    char *command;
    ListNode *args;
    Object *function;
    ListNode *list;

    assert(object->type == LIST);
    list = object->u.list;

    if (list == NULL)
        error_message("Cannot evaluate empty list.");
    else if (list->value->type != SYMBOL)
        error_message("Cannot evaluate non-symbol.");

    command = list->value->u.s;
    args = list->next;
    function = dictionary_get(dictionary, command);

    if (function == NULL)
        error_message("'%s' undefined.", command);
    else if (!object_iscallable(function))
        error_message("'%s' not a function.", command);

    return call_function(dictionary, function, args);
}
