#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "eval.h"
#include "object.h"
#include "builtins.h"
#include "util.h"
#include "dictionary.h"

Object *eval_list(Dictionary *dictionary, Object *object);

Object *eval(Dictionary *dictionary, Object *arg) {
    Object *value;

    switch (arg->type) {
        case SYMBOL:
            value = dictionary_get(dictionary, arg->u.s);
            if (value == NULL)
                error_message("'%s' undefined.", arg->u.s);
            value->refcount++;
            break;
        case CONS:
            if (is_list(arg)) {
                // Replace list with what it evluates to
                value = eval_list(dictionary, arg);
                break;
            }
            // XXX ?
            // FALLTHROUGH
        default:
            value = arg;
            value->refcount++;
    }

    return value;
}

Object *call_user_function(Dictionary *dictionary, Object *function, Object *args) {
    Dictionary *local_dictionary;
    Object *arg, *node, *argval, *nodeval, *value;

    assert(function->type == FUNCTION);
    assert(is_list(list_first(function->u.obj)));

    // Set arguments as local variables
    local_dictionary = dictionary_new(dictionary);
    node = list_first(function->u.obj);
    arg = args;
    nodeval = list_first(node);
    argval = list_first(arg);
    while (nodeval != NULL) {
        assert(nodeval->type == SYMBOL);
        if (argval == NULL)
            error_message("Wrong number of arguments to function.");

        argval->refcount++;
        dictionary_insert(local_dictionary, nodeval->u.s, argval);
        nodeval = list_next(&node);
        argval = list_next(&arg);
    }

    // Execute code; last value will be return value
    value = &NIL_CONST;
    node = function->u.obj->u.cons.cdr;
    for (nodeval=list_first(node); nodeval!=NULL; nodeval=list_next(&node)) {
        garbage_collect(value);
        value = eval(local_dictionary, nodeval);
    }

    dictionary_free(local_dictionary);

    return value;
}

Object *eval_list(Dictionary *dictionary, Object *object) {
    char *command;
    Object *args, *function;

    assert(is_list(object));

    if (object == &NIL_CONST)
        error_message("Cannot evaluate empty list.");
    else if (list_first(object)->type != SYMBOL)
        error_message("Cannot evaluate non-symbol.");

    command = list_first(object)->u.s;
    args = object->u.cons.cdr; // TODO: Better API here? 
    function = dictionary_get(dictionary, command);

    if (function == NULL)
        error_message("'%s' undefined.", command);
    else if (!object_iscallable(function))
        error_message("'%s' not a function.", command);

    return call_function(dictionary, function, args);
}

Object *map_eval(Dictionary *dictionary, Object *list) {
    Object *nodes=&NIL_CONST, *prev_node, *oldnode, *value, *oldval;

    oldnode = list;
    for (oldval=list_first(list); oldval!=NULL; oldval=list_next(&oldnode)) {
        value = eval(dictionary, oldval);
        append_node(&nodes, &prev_node, value);
    }

    return nodes;
}

Object *call_function(Dictionary *dictionary, Object *function, Object *args) {
    Object *value, *tmpnodes;
 
    switch (function->type) {
        case BUILTIN:
            tmpnodes = map_eval(dictionary, args);
            value = function->u.builtin(dictionary, tmpnodes);
            garbage_collect(tmpnodes);
            break;
        case FUNCTION:
            tmpnodes = map_eval(dictionary, args);
            value = call_user_function(dictionary, function, tmpnodes);
            garbage_collect(tmpnodes);
            break;
        case SPECIAL:
            value = function->u.builtin(dictionary, args);
            break;
        default:
            abort();
    }

    return value;
}
