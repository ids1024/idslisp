#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "eval.h"
#include "object.h"
#include "builtins.h"
#include "util.h"
#include "dictionary.h"
#include "sequence.h"

Object *eval_list(Dictionary *dictionary, Object *object);

Object *eval(Dictionary *dictionary, Object *arg) {
    Object *value;

    if (arg->type == SYMBOL) {
        value = ref(dictionary_get(dictionary, arg->u.s));
        if (value == NULL)
            error_message("'%s' undefined.", arg->u.s);
    } else if (is_list(arg)) {
        // Replace list with what it evluates to
        value = eval_list(dictionary, arg);
    } else {
        value = ref(arg);
    }

    return value;
}

Object *eval_progn(Dictionary *dictionary, Object *nodes) {
    Object *nodeval, *value;
    Iter iter;

    value = &NIL_CONST;

    iter = seq_iter(nodes);
    while ((nodeval=iter_next(&iter)) != NULL) {
        garbage_collect(value);
        value = eval(dictionary, nodeval);
    }

    return value;
}

Object *call_user_function(Dictionary *dictionary, Object *function, Object *args) {
    Dictionary *local_dictionary;
    Object *argval, *valueval, *value;
    Iter argiter, valiter;

    assert(function->type == FUNCTION);
    assert(is_list(seq_nth(function->u.obj, 0)));

    // Set arguments as local variables
    local_dictionary = dictionary_new(dictionary);
    argiter = seq_iter(args);
    valiter = seq_iter(seq_nth(function->u.obj, 0));
    while ((valueval=iter_next(&valiter)) != NULL) {
        assert(valueval->type == SYMBOL);
        if ((argval=iter_next(&argiter)) == NULL)
            error_message("Wrong number of arguments to function.");
        dictionary_insert(local_dictionary, valueval->u.s, ref(argval));
    }

    // Execute code; last value will be return value
    value = eval_progn(local_dictionary, function->u.obj->u.cons.cdr);

    dictionary_free(local_dictionary);

    return value;
}

Object *eval_list(Dictionary *dictionary, Object *object) {
    char *command;
    Object *args, *function;

    assert(is_list(object));

    if (object == &NIL_CONST)
        error_message("Cannot evaluate empty list.");
    else if (seq_nth(object, 0)->type != SYMBOL)
        error_message("Cannot evaluate non-symbol.");

    command = seq_nth(object, 0)->u.s;
    args = object->u.cons.cdr; // TODO: Better API here? 
    function = dictionary_get(dictionary, command);

    if (function == NULL)
        error_message("'%s' undefined.", command);
    else if (!object_iscallable(function))
        error_message("'%s' not a function.", command);

    return call_function(dictionary, function, args);
}

Object *map_eval(Dictionary *dictionary, Object *list) {
    Object *nodes=&NIL_CONST, *prev_node, *value, *oldval;
    Iter olditer;

    olditer = seq_iter(list);
    while ((oldval=iter_next(&olditer)) != NULL) {
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
