#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "builtins.h"
#include "util.h"
#include "eval.h"
#include "parse.h"
#include "dictionary.h"


void _args_num(char *name, Object *args, int num) {
    if (list_len(args) != num)
        error_message("Wrong number of arguments to '%s'.", name);
}


#define _OPERATOR_BUILTIN(operator, args) ({ \
    double dvalue; \
    long int ivalue; \
    bool isdouble=false; \
    Object *object; \
    Object *value, *node=args; \
    for (value=list_first(node); value!=NULL; value=list_next(&node)) { \
        switch (value->type) { \
            case INT: \
                if (node == args) \
                    dvalue = ivalue = value->u.ld; \
                else { \
                    dvalue operator value->u.ld; \
                    ivalue operator value->u.ld; \
                } \
                break; \
            case DOUBLE: \
                isdouble = true; \
                if (node == args) \
                    dvalue = value->u.lf; \
                else \
                    dvalue operator value->u.lf; \
                break; \
            default: \
                error_message("Invalid argument"); \
        } \
    } \
    \
    if (isdouble) \
        object = new_double(dvalue); \
    else \
        object = new_int(ivalue); \
    object; \
})

#define _COMPARISON_BUILTIN(operator, args) ({ \
    double num, newnum; \
    bool result=true; \
    Object *value, *node=args; \
    \
    if (list_len(args) < 1) \
        error_message("Wrong number of arguments."); \
    \
    for (value=list_first(node); value!=NULL; value=list_next(&node)) { \
        switch (value->type) { \
            case INT: \
                newnum = value->u.ld; \
                break; \
            case DOUBLE: \
                newnum = value->u.lf; \
                break; \
            default: \
                error_message("Invalid argument"); \
        } \
        if (node == args) \
            num = newnum; \
        else if (!(num operator newnum)) \
            result = false; \
    } \
    \
    from_bool(result); \
})

Object *builtin_add(Dictionary *dictionary, Object *args) {
    return _OPERATOR_BUILTIN(+=, args);
}

Object *builtin_minus(Dictionary *dictionary, Object *args) {
    return _OPERATOR_BUILTIN(-=, args);
}

Object *builtin_times(Dictionary *dictionary, Object *args) {
    return _OPERATOR_BUILTIN(*=, args);
}

Object *builtin_divide(Dictionary *dictionary, Object *args) {
    return _OPERATOR_BUILTIN(/=, args);
}

Object *builtin_equal(Dictionary *dictionary, Object *args) {
    return _COMPARISON_BUILTIN(==, args);
}

Object *builtin_greater(Dictionary *dictionary, Object *args) {
    return _COMPARISON_BUILTIN(>, args);
}

Object *builtin_less(Dictionary *dictionary, Object *args) {
    return _COMPARISON_BUILTIN(<, args);
}

Object *builtin_greater_equal(Dictionary *dictionary, Object *args) {
    return _COMPARISON_BUILTIN(>=, args);
}

Object *builtin_less_equal(Dictionary *dictionary, Object *args) {
    return _COMPARISON_BUILTIN(<=, args);
}

Object *builtin_not(Dictionary *dictionary, Object *args) {
    _args_num("not", args, 1);
    return from_bool(!to_bool(list_first(args)));
}

Object *builtin_or(Dictionary *dictionary, Object *args) {
    Object *value, *node=args;
    for (value=list_first(node); value!=NULL; value=list_next(&node)) {
        if (to_bool(value))
            return ref(value);
    }
    return &NIL_CONST;
}

Object *builtin_and(Dictionary *dictionary, Object *args) {
    Object *value, *node=args;
    for (value=list_first(node); value!=NULL; value=list_next(&node)) {
        if (!to_bool(value))
            return ref(value);
    }
    return &T_CONST;
}

Object *builtin_print(Dictionary *dictionary, Object *args) {
    Object *value, *node=args;
    value=list_first(node);
    while (value != NULL) {
        if (value->type == STRING)
            printf("%s", value->u.s);
        else
            object_print(value);

        value = list_next(&node);
        if (value != NULL)
            printf(" ");
    }
    return &NIL_CONST;
}

Object *builtin_println(Dictionary *dictionary, Object *args) {
    Object *value = builtin_print(dictionary, args);
    printf("\n");
    return value;
}

Object *builtin_list(Dictionary *dictionary, Object *args) {
    return ref(args);
}

Object *builtin_first(Dictionary *dictionary, Object *args) {
    Object *object;

    _args_num("first", args, 1);
    object = list_first(args);

    if (!is_list(object))
        error_message("Argument to 'first' must be list.");
    else if (object == &NIL_CONST)
        return &NIL_CONST;
    else
        return ref(list_first(object));
}

Object *builtin_eval(Dictionary *dictionary, Object *args) {
    Object *object;

    _args_num("eval", args, 1);
    object = list_first(args);

    if (!is_list(object))
        error_message("Argument to 'eval' must be list.");

    return eval(dictionary, object);
}

Object *builtin_map(Dictionary *dictionary, Object *args) {
    Object *function, *value, *item, *itemval, *nodes=&NIL_CONST, *prev, *newargs;

    _args_num("map", args, 2);
    if (!(object_iscallable(list_first(args))))
        error_message("First argument to 'map' must be callable.");
    else if (!is_list(list_nth(args, 1)))
        error_message("Second argument to 'map' must be list.");

    function = list_first(args);

    item = list_nth(args, 1);
    for (itemval=list_first(item); itemval!=NULL; itemval=list_next(&item)) {
        newargs = new_cons(ref(itemval), &NIL_CONST);
        value = call_function(dictionary, function, newargs); 
        garbage_collect(newargs);
        append_node(&nodes, &prev, value);
    }

    return nodes;
}

Object *builtin_mapcar(Dictionary *dictionary, Object *args) {
    Object *function, **lists, *list, *node, *value;
    Object *tmpargs=&NIL_CONST, *lastarg, *results=&NIL_CONST, *lastres;
    int i, nlists;
    bool first = true;

    if (list_len(args) < 3)
        error_message("Wrong number of arguments to 'mapcar'.");
    else if (!object_iscallable(list_first(args)))
        error_message("First argument to 'mapcar' must be callable.");

    nlists = (list_len(args) - 1);
    function = list_first(args);

    lists = malloc(nlists * sizeof(Object*));

    node = args->u.cons.cdr;
    i = 0;
    for (list=list_first(node); list!=NULL; list=list_next(&node)) {
        if (!is_list(list))
            error_message("Argument to 'mapcar' must be list.");
        lists[i] = list;
        i++;
    }

    for (;;) {
        tmpargs = &NIL_CONST;
        for (i=0; i<nlists; i++) {
            if (first)
                value = list_first(lists[i]);
            else
                value = list_next(&lists[i]);

            if (value == NULL)
                break;
            append_node(&tmpargs, &lastarg, ref(value));
        }

        if (value == NULL) {
            garbage_collect(tmpargs);
            break;
        }

        value = call_function(dictionary, function, tmpargs); 
        garbage_collect(tmpargs);
        append_node(&results, &lastres, value);
        first = false;
    }

    free(lists);

    return results;
}

Object *builtin_nth(Dictionary *dictionary, Object *args) {
    Object *value, *list;
    int index;

    _args_num("nth", args, 2);
    if (list_first(args)->type != INT)
        error_message("First argument to 'nth' must be integer.");
    else if (!is_list(list_nth(args, 1)))
        error_message("Second argument to 'nth' must be list.");

    index = list_first(args)->u.ld;
    list = list_nth(args, 1);
    value = list_nth(list, index);
    if (value == NULL) {
        return &NIL_CONST;
    } else {
        return ref(value);
    }
}

Object *builtin_read_line(Dictionary *dictionary, Object *args) {
    char *line=NULL;
    size_t n=0;
    ssize_t len;

    _args_num("read-line", args, 0);
    len = getline(&line, &n, stdin);
    line[len-1] = '\0'; // Remove newline
    return new_string(line);
}

Object *builtin_read(Dictionary *dictionary, Object *args) {
    Object *value, **objects;
    int nobjects, i;
    char *line=NULL;
    size_t n=0;
    ssize_t len;

    _args_num("read", args, 0);
    len = getline(&line, &n, stdin);
    line[len-1] = '\0'; // Remove newline

    objects = parse(line, &nobjects);
    if (nobjects == 0)
        error_message("No parsable input.");
    value = objects[0];

    // TODO: Multiple return values?
    // https://www.cs.cmu.edu/Groups/AI/html/cltl/clm/node93.html

    for (i = 1; i < nobjects; i++)
        garbage_collect(objects[i]);
    free(objects);

    return value;
}

Object *builtin_cons(Dictionary *dictionary, Object *args) {
    Object *car, *cdr;

    _args_num("cons", args, 2);
    car = list_first(args);
    cdr = list_nth(args, 1);
    return new_cons(ref(car), ref(cdr));
}

Object *builtin_car(Dictionary *dictionary, Object *args) {
    _args_num("car", args, 1);
    if (!is_list(list_first(args)))
        error_message("First argument to 'car' must be list.");

    return list_first(args)->u.cons.car;
}

Object *builtin_cdr(Dictionary *dictionary, Object *args) {
    _args_num("cdr", args, 1);
    if (!is_list(list_first(args)))
        error_message("First argument to 'cdr' must be list.");

    return list_first(args)->u.cons.cdr;
}

Object *builtin_def(Dictionary *dictionary, Object *args) {
    char *name;
    Object *value;

    _args_num("def", args, 2);
    if (list_first(args)->type != SYMBOL)
        error_message("First argument to 'def' must be symbol.");

    name = list_first(args)->u.s;
    value = eval(dictionary, list_nth(args, 1));

    dictionary_insert(dictionary, name, value);
    return new_symbol(name);
}

Object *builtin_quote(Dictionary *dictionary, Object *args) {
    _args_num("quote", args, 1);
    return ref(list_first(args));
}

Object *builtin_progn(Dictionary *dictionary, Object *args) {
    return eval_progn(dictionary, args);
}

Object *builtin_defun(Dictionary *dictionary, Object *args) {
    char *name;

    if (list_len(args) < 2)
        error_message("Wrong number of arguments to 'defun'.");
    else if (list_first(args)->type != SYMBOL)
        error_message("First argument to 'defun' must be symbol.");
    else if (!is_list(list_nth(args, 1)))
        error_message("Second argument to 'defun' must be list.");

    // TODO: Verify correctness of formatting
    
    name = list_first(args)->u.s;
    // TODO: Better API here? 
    dictionary_insert(dictionary, name, new_function(ref(args->u.cons.cdr)));
    return new_symbol(name);
}

Object *builtin_if(Dictionary *dictionary, Object *args) {
    Object *condition, *value;

    _args_num("if", args, 3);

    condition = eval(dictionary, list_first(args));
    if (to_bool(condition))
        value = eval(dictionary, list_nth(args, 1));
    else
        value = eval(dictionary, list_nth(args, 2));
    garbage_collect(condition);

    return value;
}

void builtins_load(Dictionary *dictionary) {
    dictionary_insert(dictionary, "nil", &NIL_CONST);
    dictionary_insert(dictionary, "T", &T_CONST);

    dictionary_insert(dictionary, "+", new_builtin(builtin_add));
    dictionary_insert(dictionary, "-", new_builtin(builtin_minus));
    dictionary_insert(dictionary, "*", new_builtin(builtin_times));
    dictionary_insert(dictionary, "/", new_builtin(builtin_divide));
    dictionary_insert(dictionary, "=", new_builtin(builtin_equal));
    dictionary_insert(dictionary, ">", new_builtin(builtin_greater));
    dictionary_insert(dictionary, "<", new_builtin(builtin_less));
    dictionary_insert(dictionary, ">=", new_builtin(builtin_greater_equal));
    dictionary_insert(dictionary, "<=", new_builtin(builtin_less_equal));
    dictionary_insert(dictionary, "not", new_builtin(builtin_not));
    dictionary_insert(dictionary, "or", new_builtin(builtin_or));
    dictionary_insert(dictionary, "and", new_builtin(builtin_and));
    dictionary_insert(dictionary, "print", new_builtin(builtin_print));
    dictionary_insert(dictionary, "println", new_builtin(builtin_println));
    dictionary_insert(dictionary, "list", new_builtin(builtin_list));
    dictionary_insert(dictionary, "first", new_builtin(builtin_first));
    dictionary_insert(dictionary, "eval", new_builtin(builtin_eval));
    dictionary_insert(dictionary, "map", new_builtin(builtin_map));
    dictionary_insert(dictionary, "mapcar", new_builtin(builtin_mapcar));
    dictionary_insert(dictionary, "nth", new_builtin(builtin_nth));
    dictionary_insert(dictionary, "read-line", new_builtin(builtin_read_line));
    dictionary_insert(dictionary, "read", new_builtin(builtin_read));
    dictionary_insert(dictionary, "cons", new_builtin(builtin_cons));
    dictionary_insert(dictionary, "car", new_builtin(builtin_car));
    dictionary_insert(dictionary, "cdr", new_builtin(builtin_cdr));

    dictionary_insert(dictionary, "def", new_special(builtin_def));
    dictionary_insert(dictionary, "quote", new_special(builtin_quote));
    dictionary_insert(dictionary, "progn", new_special(builtin_progn));
    dictionary_insert(dictionary, "defun", new_special(builtin_defun));
    dictionary_insert(dictionary, "if", new_special(builtin_if));
}
