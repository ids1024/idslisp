#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "object.h"

Object NIL_CONST = {NIL};
Object T_CONST = {T};

#define _NEW_OBJECT(typename, utype, value) ({ \
        Object *object = malloc(sizeof(Object)); \
        object->type = typename; \
        object->u.utype = value; \
        object->refcount = 1; \
        object; \
    })


ListNode *new_node(ListNode *next, Object *value) {
    ListNode *node = malloc(sizeof(ListNode));
    node->value = value;
    node->next = next;
    node->refcount = 1;
    return node;
}

void append_node(ListNode **list, ListNode **prev, Object *value) {
    ListNode *node = new_node(NULL, value);
    if (*list == NULL)
        *list = node;
    else
        (*prev)->next = node;
    *prev = node;
}

int list_len(ListNode *nodes) {
    int count = 0;
    ListNode *node;
    for (node=nodes; node!=NULL; node=node->next) count++;
    return count;
}

Object *list_nth(ListNode *nodes, int n) {
    int index = 0;
    ListNode *node;
    for (node=nodes; node!=NULL; node=node->next) {
        if (index == n)
            return node->value;
        index++;
    }

    return NULL;
}

Object *new_int(long int value) {
    return _NEW_OBJECT(INT, ld, value);
}

Object *new_double(double value) {
    return _NEW_OBJECT(DOUBLE, lf, value);
}

Object *new_string(char *value) {
    return _NEW_OBJECT(STRING, s, strdup(value));
}

Object *new_symbol(char *value) {
    return _NEW_OBJECT(SYMBOL, s, strdup(value));
}

Object *new_list(ListNode *value) {
    return _NEW_OBJECT(LIST, list, value);
}

Object *new_function(ListNode *value) {
    return _NEW_OBJECT(FUNCTION, list, value);
}

Object *new_builtin(BuiltinFunc value) {
    return _NEW_OBJECT(BUILTIN, builtin, value);
}

Object *new_special(BuiltinFunc value) {
    return _NEW_OBJECT(SPECIAL, builtin, value);
}

bool object_iscallable(Object *object) {
    return (object->type == BUILTIN || object->type == FUNCTION || \
            object->type == SPECIAL);
}

bool object_issingleton(Object *object) {
    return (object == &NIL_CONST || object == &T_CONST);
}

void garbage_collect(Object *object) {
    if (object_issingleton(object))
        return;

    object->refcount--;
    assert(object->refcount >= 0);

    if (object->refcount == 0) {
        switch (object->type) {
            case LIST:
            case FUNCTION:
                garbage_collect_list(object->u.list);
                break;
            case STRING:
            case SYMBOL:
                free(object->u.s);
                break;
        }
        free(object);
    }
}

void garbage_collect_list(ListNode *list) {
    ListNode *old_node, *node;

    node = list;
    while (node != NULL) {
        node->refcount--;
        assert(node->refcount >= 0);
        if (node->refcount != 0)
            break;

        garbage_collect(node->value);
        old_node = node;
        node=node->next;
        free(old_node);
    }
}

void object_print(Object *object) {
    ListNode *node;

    switch (object->type) {
        case INT:
            printf("%d", object->u.ld);
            break;
        case DOUBLE:
            printf("%lf", object->u.lf);
            break;
	case STRING:
            printf("\"%s\"", object->u.s);
	    break;
        case SYMBOL:
            printf("%s", object->u.s);
	    break;
        case LIST:
            printf("(");
            node = object->u.list;
            while (node != NULL) {
                object_print(node->value);
                node = node->next;
                if (node != NULL) {
                    printf(" ");
                }
            }
            printf(")");
            break;
        case BUILTIN:
            printf("<built-in function>");
            break;
        case SPECIAL:
            printf("<built-in special form>");
            break;
        case FUNCTION:
            printf("<user-defined function>");
            break;
        case NIL:
            printf("nil");
            break;
        case T:
            printf("T");
            break;
    }
}
