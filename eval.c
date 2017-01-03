#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "builtins.h"
#include "dictionary.h"
#include "eval.h"
#include "object.h"
#include "sequence.h"
#include "util.h"

Object *_eval_list(Dictionary *dictionary, Object *object);

Object *eval(Dictionary *dictionary, Object *arg) {
    Object *value;

    if (arg->type == SYMBOL) {
        value = ref(dictionary_get(dictionary, arg->u.s));
        if (value == NULL)
            error_message("'%s' undefined.", arg->u.s);
    } else if (is_list(arg)) {
        // Replace list with what it evluates to
        value = _eval_list(dictionary, arg);
    } else {
        value = ref(arg);
    }

    return value;
}

Object *eval_progn(Dictionary *dictionary, Object *nodes) {
    Object *value = &NIL_CONST;

    TRY_START(1, value);

    Iter iter = seq_iter(nodes);
    Object *nodeval;
    while ((nodeval = iter_next(&iter)) != NULL) {
        garbage_collect(value);
        value = eval(dictionary, nodeval);
    }

    TRY_END();
    return value;
}

Object *call_user_function(Dictionary *dictionary, Object *function,
                           Object *args) {
    assert(function->type == FUNCTION);
    assert(is_list(seq_nth(function->u.obj, 0)));

    // Set arguments as local variables
    Dictionary *local_dictionary = dictionary_new(dictionary);
    Iter argiter = seq_iter(args);
    Iter valiter = seq_iter(seq_nth(function->u.obj, 0));
    Object *valueval;
    while ((valueval = iter_next(&valiter)) != NULL) {
        assert(valueval->type == SYMBOL);
        Object *argval;
        if ((argval = iter_next(&argiter)) == NULL)
            error_message("Wrong number of arguments to function.");
        dictionary_insert(local_dictionary, valueval->u.s, ref(argval));
    }

    // Execute code; last value will be return value
    Object *value = eval_progn(local_dictionary, function->u.obj->u.cons.cdr);

    dictionary_free(local_dictionary);

    return value;
}

Object *_eval_list(Dictionary *dictionary, Object *object) {
    assert(is_list(object));

    if (object == &NIL_CONST)
        error_message("Cannot evaluate empty list.");
    else if (seq_nth(object, 0)->type != SYMBOL)
        error_message("Cannot evaluate non-symbol.");

    char *command = seq_nth(object, 0)->u.s;
    Object *args = object->u.cons.cdr; // TODO: Better API here?
    Object *function = dictionary_get(dictionary, command);

    if (function == NULL)
        error_message("'%s' undefined.", command);
    else if (!object_iscallable(function))
        error_message("'%s' not a function.", command);

    return call_function(dictionary, function, args);
}

Object *map_eval(Dictionary *dictionary, Object *list) {
    Iter olditer = seq_iter(list);
    Object *nodes = &NIL_CONST, *prev_node, *oldval;
    while ((oldval = iter_next(&olditer)) != NULL) {
        Object *value = eval(dictionary, oldval);
        append_node(&nodes, &prev_node, value);
    }

    return nodes;
}

Object *call_function(Dictionary *dictionary, Object *function, Object *args) {
    Object *value, *tmpnodes = NULL;

    TRY_START(1, tmpnodes);

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

    TRY_END()
    return value;
}
