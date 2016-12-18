#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "object.h"


#define _NEW_OBJECT(typename, utype, value) ({ \
        Object *object = malloc(sizeof(Object)); \
        object->type = typename; \
        object->u.utype = value; \
        object; \
    })


ListNode *new_node(ListNode *next, Object *value) {
    ListNode *node = malloc(sizeof(ListNode));
    node->value = value;
    node->next = next;
    return node;
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

void list_prepend(Object *object, Object *value) {
    assert(object->type == LIST);
    object->u.list = new_node(object->u.list, value);
}

Object *object_free(Object *object) {
    ListNode *node;

    switch (object->type) {
        case LIST:
            node = object->u.list;
            while (node != NULL) {
                node = object->u.list;
                free(node->value);
                free(node);
            }
            break;
        case STRING:
        case SYMBOL:
            free(object->u.s);
            break;
    }
    free(object);
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
    }
}
