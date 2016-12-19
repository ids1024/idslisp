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

int _count_args(ListNode *args) {
    int count = 0;
    ListNode *arg;
    for (arg=args; arg!=NULL; arg=arg->next) count++;
    return count;
}

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
    return new_nil();
}

Object *builtin_println(Dictionary *dictionary, ListNode *args) {
    Object *value = builtin_print(dictionary, args);
    printf("\n");
    return value;
}

Object *builtin_list(Dictionary *dictionary, ListNode *args) {
    return new_list(args);
}

Object *builtin_first(Dictionary *dictionary, ListNode *args) {
    if (_count_args(args) != 1)
        error_message("Wrong number of arguments to 'first'.");
    else if (args->value->type != LIST)
        error_message("Argument to 'first' must be list.");
    else if (args->value->u.list == NULL)
        return new_nil();
    else
        return args->value->u.list->value;
}

Object *builtin_def(Dictionary *dictionary, ListNode *args) {
    char *name;
    Object *value;

    if (_count_args(args) != 2)
        error_message("Wrong number of arguments to 'def'.");
    else if (args->value->type != SYMBOL)
        error_message("First argument to 'def' must be symbol.");

    name = args->value->u.s;
    value = eval_arg(dictionary, args->next->value);

    dictionary_insert(dictionary, name, value);
    return new_symbol(name);
}

Object *builtin_quote(Dictionary *dictionary, ListNode *args) {
    if (_count_args(args) != 1)
        error_message("Wrong number of arguments to 'quote'.");

    return args->value;
}

void builtins_load(Dictionary *dictionary) {
    dictionary_insert(dictionary, "+", new_builtin(builtin_add));
    dictionary_insert(dictionary, "-", new_builtin(builtin_minus));
    dictionary_insert(dictionary, "*", new_builtin(builtin_times));
    dictionary_insert(dictionary, "/", new_builtin(builtin_divide));
    dictionary_insert(dictionary, "print", new_builtin(builtin_print));
    dictionary_insert(dictionary, "println", new_builtin(builtin_println));
    dictionary_insert(dictionary, "list", new_builtin(builtin_list));
    dictionary_insert(dictionary, "first", new_builtin(builtin_first));
    dictionary_insert(dictionary, "def", new_special(builtin_def));
    dictionary_insert(dictionary, "quote", new_special(builtin_quote));
}
