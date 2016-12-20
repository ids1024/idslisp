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
    \
    for (ListNode *node=args; node!=NULL; node=node->next) { \
        switch (node->value->type) { \
            case INT: \
                if (node == args) \
                    dvalue = ivalue = node->value->u.ld; \
                else { \
                    dvalue operator node->value->u.ld; \
                    ivalue operator node->value->u.ld; \
                } \
                break; \
            case DOUBLE: \
                isdouble = true; \
                if (node == args) \
                    dvalue = node->value->u.lf; \
                else \
                    dvalue operator node->value->u.lf; \
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

Object *builtin_add(Dictionary *dictionary, ListNode *args) {
    return _OPERATOR_BUILTIN(+=, args);
}

Object *builtin_minus(Dictionary *dictionary, ListNode *args) {
    return _OPERATOR_BUILTIN(-=, args);
}

Object *builtin_times(Dictionary *dictionary, ListNode *args) {
    return _OPERATOR_BUILTIN(*=, args);
}

Object *builtin_divide(Dictionary *dictionary, ListNode *args) {
    return _OPERATOR_BUILTIN(/=, args);
}

Object *builtin_print(Dictionary *dictionary, ListNode *args) {
    ListNode *arg;

    arg = args;
    while (arg!=NULL) {
        if (arg->value->type == STRING)
            printf("%s", arg->value->u.s);
        else
            object_print(arg->value);

        arg = arg->next;
        if (arg != NULL)
            printf(" ");
    }
    return &NIL_CONST;
}

Object *builtin_println(Dictionary *dictionary, ListNode *args) {
    Object *value = builtin_print(dictionary, args);
    printf("\n");
    return value;
}

Object *builtin_list(Dictionary *dictionary, ListNode *args) {
    args->refcount++;
    return new_list(args);
}

Object *builtin_first(Dictionary *dictionary, ListNode *args) {
    if (list_len(args) != 1)
        error_message("Wrong number of arguments to 'first'.");
    else if (args->value->type != LIST)
        error_message("Argument to 'first' must be list.");
    else if (args->value->u.list == NULL)
        return &NIL_CONST;
    else {
        args->value->u.list->value->refcount++;
        return args->value->u.list->value;
    }
}

Object *builtin_eval(Dictionary *dictionary, ListNode *args) {
    if (list_len(args) != 1)
        error_message("Wrong number of arguments to 'eval'.");
    else if (args->value->type != LIST)
        error_message("Argument to 'eval' must be list.");

    return eval(dictionary, args->value);
}

Object *builtin_map(Dictionary *dictionary, ListNode *args) {
    Object *function, *value;
    ListNode *item, *nodes=NULL, *prev_node, *newargs;

    if (list_len(args) != 2)
        error_message("Wrong number of arguments to 'map'.");
    else if (!(object_iscallable(args->value)))
        error_message("First argument to 'map' must be callable.");
    else if (list_nth(args, 1)->type != LIST)
        error_message("Second argument to 'map' must be list.");

    function = args->value;

    item = list_nth(args, 1)->u.list;
    while (item != NULL) {
        item->value->refcount++;
        newargs = new_node(NULL, item->value);
        value = call_function(dictionary, function, newargs); 
        garbage_collect_list(newargs);
        append_node(&nodes, &prev_node, value);
        item = item->next;
    }

    return new_list(nodes);
}

Object *builtin_nth(Dictionary *dictionary, ListNode *args) {
    Object *value;
    ListNode *list;
    int index;

    if (list_len(args) != 2)
        error_message("Wrong number of arguments to 'nth'.");
    else if (args->value->type != INT)
        error_message("First argument to 'nth' must be integer.");
    else if (list_nth(args, 1)->type != LIST)
        error_message("Second argument to 'nth' must be list.");

    index = args->value->u.ld;
    list = list_nth(args, 1)->u.list;
    value = list_nth(list, index);
    if (value == NULL) {
        return &NIL_CONST;
    } else {
        value->refcount++;
        return value;
    }
}

Object *builtin_def(Dictionary *dictionary, ListNode *args) {
    char *name;
    Object *value;

    if (list_len(args) != 2)
        error_message("Wrong number of arguments to 'def'.");
    else if (args->value->type != SYMBOL)
        error_message("First argument to 'def' must be symbol.");

    name = args->value->u.s;
    value = eval_arg(dictionary, list_nth(args, 1));

    dictionary_insert(dictionary, name, value);
    return new_symbol(name);
}

Object *builtin_quote(Dictionary *dictionary, ListNode *args) {
    if (list_len(args) != 1)
        error_message("Wrong number of arguments to 'quote'.");

    args->value->refcount++;
    return args->value;
}

Object *builtin_defun(Dictionary *dictionary, ListNode *args) {
    Object *function;
    char *name;

    if (list_len(args) < 2)
        error_message("Wrong number of arguments to 'defun'.");
    else if (args->value->type != SYMBOL)
        error_message("First argument to 'defun' must be symbol.");
    else if (list_nth(args, 1)->type != LIST)
        error_message("Second argument to 'defun' must be list.");

    // TODO: Verify correctness of formatting
    
    args->next->refcount++;
    name = args->value->u.s;
    dictionary_insert(dictionary, name, new_function(args->next));
    return new_symbol(name);
}

Object *builtin_if(Dictionary *dictionary, ListNode *args) {
    Object *condition, *value;

    if (list_len(args) != 3)
        error_message("Wrong number of arguments to 'if'.");

    condition = eval_arg(dictionary, args->value);
    if (condition != &NIL_CONST)
        value = eval_arg(dictionary, list_nth(args, 1));
    else
        value = eval_arg(dictionary, list_nth(args, 2));
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
    dictionary_insert(dictionary, "print", new_builtin(builtin_print));
    dictionary_insert(dictionary, "println", new_builtin(builtin_println));
    dictionary_insert(dictionary, "list", new_builtin(builtin_list));
    dictionary_insert(dictionary, "first", new_builtin(builtin_first));
    dictionary_insert(dictionary, "eval", new_builtin(builtin_eval));
    dictionary_insert(dictionary, "map", new_builtin(builtin_map));
    dictionary_insert(dictionary, "nth", new_builtin(builtin_nth));

    dictionary_insert(dictionary, "def", new_special(builtin_def));
    dictionary_insert(dictionary, "quote", new_special(builtin_quote));
    dictionary_insert(dictionary, "defun", new_special(builtin_defun));
    dictionary_insert(dictionary, "if", new_special(builtin_if));
}
