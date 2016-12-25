#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "object.h"

Object NIL_CONST = {NIL};
Object T_CONST = {T};

#define _NEW_OBJECT(typename, utype, ...) ({ \
        Object *object = malloc(sizeof(Object)); \
        *object = (Object){typename, {.utype = __VA_ARGS__}, 1}; \
        object; \
    })


void append_node(Object **list, Object **prev, Object *value) {
    Object *node = new_cons(value, &NIL_CONST);
    if (*list == &NIL_CONST)
        *list = node;
    else {
	assert((*prev)->type == CONS);
        (*prev)->u.cons.cdr = node;
    }
    *prev = node;
}

bool is_list(Object *object) {
    return(((object->type == CONS) && \
            (object->u.cons.cdr == &NIL_CONST || \
             object->u.cons.cdr->type == CONS)) || \
           (object == &NIL_CONST));
}

Object *list_first(Object *object) {
    assert(is_list(object));
    return (object == &NIL_CONST) ? NULL : object->u.cons.car;
}

Object *list_next(Object **object) {
    Object *value;

    if (*object == NULL)
        return NULL;

    assert(is_list(*object) && *object != &NIL_CONST);
    *object = (*object)->u.cons.cdr;

    if (!is_list(*object)) {
        value = *object;
        *object = NULL;
        return value;
    } else
        return list_first(*object);
}

int list_len(Object *nodes) {
    int count = 0;
    Object *value, *node=nodes;
    for (value=list_first(nodes); value!=NULL; value=list_next(&node))
        count++;
    return count;
}

Object *list_nth(Object *nodes, int n) {
    int index = 0;
    Object *value, *node=nodes;
    assert(is_list(node));
    for (value=list_first(nodes); value!=NULL; value=list_next(&node)) {
        if (index == n)
            return value;
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

Object *new_cons(Object *car, Object *cdr) {
    return _NEW_OBJECT(CONS, cons, {car, cdr});
}

Object *new_function(Object *value) {
    return _NEW_OBJECT(FUNCTION, obj, value);
}

Object *new_builtin(BuiltinFunc value) {
    return _NEW_OBJECT(BUILTIN, builtin, value);
}

Object *new_special(BuiltinFunc value) {
    return _NEW_OBJECT(SPECIAL, builtin, value);
}

Object *from_bool(bool value) {
    return (value) ? &T_CONST : &NIL_CONST;
}

bool to_bool(Object *value) {
    return (value != &NIL_CONST);
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
            case CONS:
                garbage_collect(object->u.cons.car);
                garbage_collect(object->u.cons.cdr);
                break;
            case FUNCTION:
                garbage_collect(object->u.obj);
                break;
            case STRING:
            case SYMBOL:
                free(object->u.s);
                break;
            default:
                break;
        }
        free(object);
    }
}

void _print_cons(Object *object) {
    Object *value, *node;

    assert(object->type == CONS);
    if (is_list(object)) {
        printf("(");
        node = object;
        value = list_first(node);
        while (value != NULL) {
            object_print(value);
            value = list_next(&node);
            if (value != NULL) {
                printf(" ");
            }
        }
        printf(")");
    } else {
        printf("(");
        object_print(object->u.cons.car);
        printf(" . ");
        object_print(object->u.cons.cdr);
        printf(")");
    }
}

void object_print(Object *object) {
    switch (object->type) {
        case INT:
            printf("%ld", object->u.ld);
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
        case CONS:
            _print_cons(object);
            break;
        case BUILTIN:
        case SPECIAL:
        case FUNCTION:
            printf("<%s>", type_name(object->type));
            break;
        case NIL:
            printf("nil");
            break;
        case T:
            printf("T");
            break;
        case LIST:
        case PAIR:
            abort();
    }
}

Type object_type(Object *object) {
    if (object->type == CONS)
    	return is_list(object) ? LIST : PAIR;
    else
        return object->type;
}

char *type_name(Type type) {
    switch(type) {
        case INT:
            return "int";
        case DOUBLE:
            return "double";
        case STRING:
            return "string";
        case SYMBOL:
            return "symbol";
        case CONS:
            return "cons";
        case BUILTIN:
            return "built-in function";
        case NIL:
            return "nil";
        case SPECIAL:
            return "built-in special form";
        case FUNCTION:
            return "user-defined function";
        case T:
            return "T";
        case LIST:
            return "list";
        case PAIR:
            return "pair";
        default:
            abort();
    }
}
