#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdarg.h>

#include "builtins.h"
#include "util.h"
#include "eval.h"
#include "parse.h"
#include "dictionary.h"
#include "sequence.h"


void _args_num(char *name, Object *args, int num) {
    if (seq_len(args) != num)
        error_message("Wrong number of arguments to '%s'.", name);
}

void _arg_error(char *func, int n, char *correct, Object *object) {
    error_message("Argument %d to '%s' should be %s, is %s.",
            n, func, correct, type_name(object_type(object)));
}

void _args_check(char *name, Object *args, int num, ...) {
    Object *value;
    va_list vargs;
    int i=1;
    Type type;
    Iter iter;

    _args_num(name, args, num);

    va_start(vargs, num);
    iter = seq_iter(args);
    while ((value=iter_next(&iter)) != NULL) {
        type = va_arg(vargs, Type);
        if (object_type(value) != type)
            _arg_error(name, i, type_name(type), value);
        i++;
    }
    va_end(vargs);
}


#define _OPERATOR_BUILTIN(operator, args) ({ \
    double dvalue; \
    long int ivalue; \
    bool isdouble=false, first=true; \
    Object *object, *value; \
    Iter iter = seq_iter(args); \
    while ((value=iter_next(&iter)) != NULL) { \
        switch (value->type) { \
            case INT: \
                if (first) \
                    dvalue = ivalue = value->u.ld; \
                else { \
                    dvalue operator value->u.ld; \
                    ivalue operator value->u.ld; \
                } \
                break; \
            case DOUBLE: \
                isdouble = true; \
                if (first) \
                    dvalue = value->u.lf; \
                else \
                    dvalue operator value->u.lf; \
                break; \
            default: \
                error_message("Invalid argument"); \
        } \
        first = false; \
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
    Object *value; \
    Iter iter; \
    bool first = true; \
    \
    if (seq_len(args) < 1) \
        error_message("Wrong number of arguments."); \
    \
    iter = seq_iter(args); \
    while ((value=iter_next(&iter)) != NULL) { \
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
        if (first) { \
            num = newnum; \
            first = false; \
        } \
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
    return from_bool(!to_bool(seq_nth(args, 0)));
}

Object *builtin_or(Dictionary *dictionary, Object *args) {
    Object *value;
    Iter iter = seq_iter(args);
    while ((value=iter_next(&iter)) != NULL) {
        if (to_bool(value))
            return ref(value);
    }
    return &NIL_CONST;
}

Object *builtin_and(Dictionary *dictionary, Object *args) {
    Object *value;
    Iter iter = seq_iter(args);
    while ((value=iter_next(&iter)) != NULL) {
        if (!to_bool(value))
            return ref(value);
    }
    return &T_CONST;
}

Object *builtin_print(Dictionary *dictionary, Object *args) {
    Object *value;
    Iter iter = seq_iter(args);
    value = iter_next(&iter);
    while (value != NULL) {
        if (value->type == STRING)
            printf("%s", value->u.s);
        else
            object_print(value);

        value = iter_next(&iter);
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
    Object *value;

    if (!object_isseq(seq_nth(args, 0)))
        _arg_error("first", 0, "sequence", seq_nth(args, 0));

    value = seq_nth(seq_nth(args, 0), 0);

    return (value == NULL) ? &NIL_CONST : ref(value);
}

Object *builtin_eval(Dictionary *dictionary, Object *args) {
    _args_check("eval", args, 1, LIST);
    return eval(dictionary, seq_nth(args, 0));
}

Object *builtin_mapcar(Dictionary *dictionary, Object *args) {
    Object *function, *value;
    Object *tmpargs=&NIL_CONST, *lastarg, *results=&NIL_CONST, *lastres;
    int i, nlists;
    Iter iter, *lists;

    if (seq_len(args) < 2)
        error_message("Wrong number of arguments to 'mapcar'.");
    else if (!object_iscallable(seq_nth(args, 0)))
        error_message("Argument 1 to 'mapcar' must be callable.");

    nlists = (seq_len(args) - 1);
    function = seq_nth(args, 0);

    lists = malloc(nlists * sizeof(Iter));

    i = 0;
    iter = seq_iter(args);
    while ((value=iter_next(&iter)) != NULL) {
        if (!is_list(value))
            error_message("Argument to 'mapcar' must be list.");
        lists[i] = seq_iter(value);
        i++;
    }

    for (;;) {
        tmpargs = &NIL_CONST;
        for (i=0; i<nlists; i++) {
            value = iter_next(&lists[i]);
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
    }

    free(lists);

    return results;
}

Object *builtin_nth(Dictionary *dictionary, Object *args) {
    Object *value, *list;
    int index;

    if (seq_nth(args, 0)->type != INT)
        _arg_error("nth", 0, "int", seq_nth(args, 0));
    else if (!object_isseq(seq_nth(args, 1)))
        _arg_error("nth", 1, "sequence", seq_nth(args, 1));

    index = seq_nth(args, 0)->u.ld;
    list = seq_nth(args, 1);
    value = seq_nth(list, index);

    return (value == NULL) ? &NIL_CONST : ref(value);
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
    car = seq_nth(args, 0);
    cdr = seq_nth(args, 1);
    return new_cons(ref(car), ref(cdr));
}

Object *builtin_car(Dictionary *dictionary, Object *args) {
    _args_check("car", args, 1, LIST);
    return ref(seq_nth(args, 0)->u.cons.car);
}

Object *builtin_cdr(Dictionary *dictionary, Object *args) {
    _args_check("cdr", args, 1, LIST);
    return ref(seq_nth(args, 0)->u.cons.cdr);
}

Object *builtin_vector(Dictionary *dictionary, Object *args) {
    Object *value, **vector;
    int i=0;
    Iter iter;

    vector = malloc(seq_len(args) * sizeof(Object*));

    iter = seq_iter(args);
    while ((value=iter_next(&iter)) != NULL) {
        vector[i] = ref(value);
        i++;
    }

    return new_vector(vector, i);
}

Object *builtin_def(Dictionary *dictionary, Object *args) {
    char *name;
    Object *value;

    _args_num("def", args, 2);
    if (seq_nth(args, 0)->type != SYMBOL)
        error_message("Argument 1 to 'def' must be symbol.");

    name = seq_nth(args, 0)->u.s;
    value = eval(dictionary, seq_nth(args, 1));

    dictionary_insert(dictionary_top(dictionary), name, value);
    return new_symbol(name);
}

Object *builtin_quote(Dictionary *dictionary, Object *args) {
    _args_num("quote", args, 1);
    return ref(seq_nth(args, 0));
}

Object *builtin_progn(Dictionary *dictionary, Object *args) {
    return eval_progn(dictionary, args);
}

Object *builtin_let(Dictionary *dictionary, Object *args) {
    Dictionary *local_dictionary;
    Object *value;
    char *key;
    Iter iter;

    if (seq_len(args) < 2)
        error_message("Wrong number of arguments to 'let'.");
    else if (!is_list(seq_nth(args, 0)))
        error_message("Argument 1 to 'let' must be list.");

    local_dictionary = dictionary_new(dictionary);

    iter = seq_iter(args);
    while ((value=iter_next(&iter)) != NULL) {
        if (!is_list(value) || seq_len(value) != 2 || 
            seq_nth(value, 0)->type != SYMBOL)
            error_message("Improper format for 'let' command.");
        key = seq_nth(value, 0)->u.s;
        value = seq_nth(value, 1);
        dictionary_insert(local_dictionary, key, ref(value));
    }

    value = eval_progn(local_dictionary, args->u.cons.cdr);
    dictionary_free(local_dictionary);
    return value;

}

Object *builtin_defun(Dictionary *dictionary, Object *args) {
    char *name;

    if (seq_len(args) < 2)
        error_message("Wrong number of arguments to 'defun'.");
    else if (seq_nth(args, 0)->type != SYMBOL)
        error_message("Argument 1 to 'defun' must be symbol.");
    else if (!is_list(seq_nth(args, 1)))
        error_message("Argument 2 to 'defun' must be list.");

    // TODO: Verify correctness of formatting
    
    name = seq_nth(args, 0)->u.s;
    // TODO: Better API here? 
    dictionary_insert(dictionary, name, new_function(ref(args->u.cons.cdr)));
    return new_symbol(name);
}

Object *builtin_if(Dictionary *dictionary, Object *args) {
    Object *condition, *value;

    _args_num("if", args, 3);

    condition = eval(dictionary, seq_nth(args, 0));
    if (to_bool(condition))
        value = eval(dictionary, seq_nth(args, 1));
    else
        value = eval(dictionary, seq_nth(args, 2));
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
    dictionary_insert(dictionary, "mapcar", new_builtin(builtin_mapcar));
    dictionary_insert(dictionary, "nth", new_builtin(builtin_nth));
    dictionary_insert(dictionary, "read-line", new_builtin(builtin_read_line));
    dictionary_insert(dictionary, "read", new_builtin(builtin_read));
    dictionary_insert(dictionary, "cons", new_builtin(builtin_cons));
    dictionary_insert(dictionary, "car", new_builtin(builtin_car));
    dictionary_insert(dictionary, "cdr", new_builtin(builtin_cdr));
    dictionary_insert(dictionary, "vector", new_builtin(builtin_vector));

    dictionary_insert(dictionary, "def", new_special(builtin_def));
    dictionary_insert(dictionary, "quote", new_special(builtin_quote));
    dictionary_insert(dictionary, "progn", new_special(builtin_progn));
    dictionary_insert(dictionary, "let", new_special(builtin_let));
    dictionary_insert(dictionary, "defun", new_special(builtin_defun));
    dictionary_insert(dictionary, "if", new_special(builtin_if));
}
