#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "builtins.h"
#include "util.h"
#include "eval.h"
#include "dictionary.h"


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
    if (list_len(args) != 1)
        error_message("Wrong number of arguments to 'not'.");
    return from_bool(!to_bool(list_first(args)));
}

Object *builtin_or(Dictionary *dictionary, Object *args) {
    Object *value, *node=args;
    for (value=list_first(node); value!=NULL; value=list_next(&node)) {
        if (to_bool(value)) {
            value->refcount++;
            return value;
        }
    }
    return &NIL_CONST;
}

Object *builtin_and(Dictionary *dictionary, Object *args) {
    Object *value, *node=args;
    for (value=list_first(node); value!=NULL; value=list_next(&node)) {
        if (!to_bool(value)) {
            value->refcount++;
            return value;
        }
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
    args->refcount++;
    return args;
}

Object *builtin_first(Dictionary *dictionary, Object *args) {
    Object *object;

    if (list_len(args) != 1)
        error_message("Wrong number of arguments to 'first'.");

    object = list_first(args);

    if (!is_list(object))
        error_message("Argument to 'first' must be list.");
    else if (object == &NIL_CONST)
        return &NIL_CONST;
    else {
        list_first(object)->refcount++;
        return list_first(object);
    }
}

Object *builtin_eval(Dictionary *dictionary, Object *args) {
    Object *object;

    if (list_len(args) != 1)
        error_message("Wrong number of arguments to 'eval'.");

    object = list_first(args);

    if (!is_list(object))
        error_message("Argument to 'eval' must be list.");

    return eval(dictionary, object);
}

Object *builtin_map(Dictionary *dictionary, Object *args) {
    Object *function, *value, *argvalue, *arg=args, *nodes=NULL, *prev_node, *newargs;

    if (list_len(args) != 2)
        error_message("Wrong number of arguments to 'map'.");
    else if (!(object_iscallable(list_first(args))))
        error_message("First argument to 'map' must be callable.");
    else if (!is_list(list_nth(args, 1)))
        error_message("Second argument to 'map' must be list.");

    function = list_first(args);

    while ((argvalue = list_next(&arg)) != NULL) {
        argvalue->refcount++;

        newargs = new_cons(argvalue, &NIL_CONST);
        value = call_function(dictionary, function, newargs); 
        garbage_collect(newargs);
        append_node(&nodes, &prev_node, value);
    }

    return nodes;
}

Object *builtin_nth(Dictionary *dictionary, Object *args) {
    Object *value, *list;
    int index;

    if (list_len(args) != 2)
        error_message("Wrong number of arguments to 'nth'.");
    else if (list_first(args) != INT)
        error_message("First argument to 'nth' must be integer.");
    else if (!is_list(list_nth(args, 1)))
        error_message("Second argument to 'nth' must be list.");

    index = list_first(args)->u.ld;
    list = list_nth(args, 1);
    value = list_nth(list, index);
    if (value == NULL) {
        return &NIL_CONST;
    } else {
        value->refcount++;
        return value;
    }
}

Object *builtin_read_line(Dictionary *dictionary, Object *args) {
    char *line=NULL;
    size_t n=0;
    ssize_t len;

    if (list_len(args) != 0)
        error_message("Wrong number of arguments to 'read-line'.");

    len = getline(&line, &n, stdin);
    line[len-1] = '\0'; // Remove newline
    return new_string(line);
}

Object *builtin_cons(Dictionary *dictionary, Object *args) {
    Object *car, *cdr;

    if (list_len(args) != 2)
        error_message("Wrong number of arguments to 'cons'.");

    car = list_first(args);
    cdr = list_nth(args, 1);
    car->refcount++;
    cdr->refcount++;
    return new_cons(car, cdr);
}

Object *builtin_car(Dictionary *dictionary, Object *args) {
    if (list_len(args) != 1)
        error_message("Wrong number of arguments to 'car'.");
    else if (!is_list(list_first(args)))
        error_message("First argument to 'car' must be list.");

    return list_first(args)->u.cons.car;
}

Object *builtin_cdr(Dictionary *dictionary, Object *args) {
    if (list_len(args) != 1)
        error_message("Wrong number of arguments to 'cdr'.");
    else if (!is_list(list_first(args)))
        error_message("First argument to 'cdr' must be list.");

    return list_first(args)->u.cons.cdr;
}

Object *builtin_def(Dictionary *dictionary, Object *args) {
    char *name;
    Object *value;

    if (list_len(args) != 2)
        error_message("Wrong number of arguments to 'def'.");
    else if (list_first(args)->type != SYMBOL)
        error_message("First argument to 'def' must be symbol.");

    name = list_first(args)->u.s;
    value = eval(dictionary, list_nth(args, 1));

    dictionary_insert(dictionary, name, value);
    return new_symbol(name);
}

Object *builtin_quote(Dictionary *dictionary, Object *args) {
    if (list_len(args) != 1)
        error_message("Wrong number of arguments to 'quote'.");

    list_first(args)->refcount++;
    return list_first(args);
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
    args->u.cons.cdr->refcount++; // TODO: Better API here? 
    dictionary_insert(dictionary, name, new_function(args->u.cons.cdr));
    return new_symbol(name);
}

Object *builtin_if(Dictionary *dictionary, Object *args) {
    Object *condition, *value;

    if (list_len(args) != 3)
        error_message("Wrong number of arguments to 'if'.");

    condition = eval(dictionary, list_first(args));
    if (condition != &NIL_CONST)
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
    dictionary_insert(dictionary, "nth", new_builtin(builtin_nth));
    dictionary_insert(dictionary, "read-line", new_builtin(builtin_read_line));
    dictionary_insert(dictionary, "cons", new_builtin(builtin_cons));
    dictionary_insert(dictionary, "car", new_builtin(builtin_car));
    dictionary_insert(dictionary, "cdr", new_builtin(builtin_cdr));

    dictionary_insert(dictionary, "def", new_special(builtin_def));
    dictionary_insert(dictionary, "quote", new_special(builtin_quote));
    dictionary_insert(dictionary, "defun", new_special(builtin_defun));
    dictionary_insert(dictionary, "if", new_special(builtin_if));
}
