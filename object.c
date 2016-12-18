#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "object.h"


ListNode *new_node(ListNode *next, Object *value) {
    ListNode *node = malloc(sizeof(ListNode));
    node->value = value;
    node->next = next;
    return node;
}

Object *new_int(long int value) {
    Object *object = malloc(sizeof(Object));
    object->type = INT;
    object->u.ld = value;
    return object;
}

Object *new_double(double value) {
    Object *object = malloc(sizeof(Object));
    object->type = DOUBLE;
    object->u.lf = value;
    return object;
}

Object *new_string(char *value) {
    Object *object = malloc(sizeof(Object));
    object->type = STRING;
    object->u.s = strdup(value);
    return object;
}

Object *new_list(ListNode *value) {
    Object *object = malloc(sizeof(Object));
    object->type = LIST;
    object->u.list = value;
    return object;
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
