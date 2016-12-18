#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "builtins.h"


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
                printf("Invald argument\n"); \
                exit(1); \
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
