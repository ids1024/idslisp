#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "builtins.h"
#include "util.h"


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

Object *builtin_add(ListNode *args) {
    return _OPERATOR_BUILTIN(+=, args);
}

Object *builtin_minus(ListNode *args) {
    return _OPERATOR_BUILTIN(-=, args);
}

Object *builtin_times(ListNode *args) {
    return _OPERATOR_BUILTIN(*=, args);
}

Object *builtin_divide(ListNode *args) {
    return _OPERATOR_BUILTIN(/=, args);
}

Object *builtin_print(ListNode *args) {
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

Object *builtin_println(ListNode *args) {
    Object *value = builtin_print(args);
    printf("\n");
    return value;
}

Object *builtin_list(ListNode *args) {
    return new_list(args);
}

Object *builtin_first(ListNode *args) {
    if (args == NULL)
        error_message("Wrong number of arguments 'first'.");
    else if (args->value->type != LIST)
        error_message("Argument to 'first' must be list.");
    else if (args->value->u.list == NULL)
        return new_nil();
    else
        return args->value->u.list->value;
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
}
