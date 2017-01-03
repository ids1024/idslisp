#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "object.h"
#include "sequence.h"

Object NIL_CONST = {NIL}; ///< Global nil object
Object T_CONST = {T}; ///< Global T object

#define _NEW_OBJECT(typename, utype, ...) ({ \
        Object *object = malloc(sizeof(Object)); \
        *object = (Object){typename, {.utype = __VA_ARGS__}, 1}; \
        object; \
    })

/**
 * @brief Append to linked list
 * @param list Pointer to list to update
 * @param prev Pointer to previous node pointer
 * @param value Value to append
 */
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

/**
 * @brief Returns true if object is a list
 */
bool is_list(Object *object) {
    return(((object->type == CONS) && \
            (object->u.cons.cdr == &NIL_CONST || \
             object->u.cons.cdr->type == CONS)) || \
           (object == &NIL_CONST));
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

Object *new_character(char character) {
    return _NEW_OBJECT(CHARACTER, c, character);
}

Object *new_symbol(char *value) {
    return _NEW_OBJECT(SYMBOL, s, strdup(value));
}

Object *new_cons(Object *car, Object *cdr) {
    return _NEW_OBJECT(CONS, cons, {car, cdr});
}

Object *new_vector(Object **items, int nitems) {
    return _NEW_OBJECT(VECTOR, vec, {items, nitems});
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

/**
 * @brief Converts c boolean to lisp object
 */
Object *from_bool(bool value) {
    return (value) ? &T_CONST : &NIL_CONST;
}

/**
 * @brief Converts lisp object to C boolean
 */
bool to_bool(Object *value) {
    return (value != &NIL_CONST);
}

/**
 * @brief Returns true if object is callable
 */
bool object_iscallable(Object *object) {
    return (object->type == BUILTIN || object->type == FUNCTION || \
            object->type == SPECIAL);
}

/**
 * @brief Returns true if object is singleton
 *
 * Currently only nil and T are singletons.
 */
bool object_issingleton(Object *object) {
    return (object == &NIL_CONST || object == &T_CONST);
}

/**
 * @brief Garbage collect object
 *
 * This decrements the objects reference counter, and if no other references
 * to the object remain, frees it and garbage collects all objects referenced
 * by it.
 */
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
            case VECTOR:
                for (int i=0; i<object->u.vec.nitems; i++) {
                    garbage_collect(object->u.vec.items[i]);
                }
                free(object->u.vec.items);
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

void _print_seq(Object *object) {
    assert(object_isseq(object));

    Iter iter = seq_iter(object);
    Object *value = iter_next(&iter);
    while (value != NULL) {
        object_print(value);
        value = iter_next(&iter);
        if (value != NULL) {
            printf(" ");
        }
    }
}

void _print_cons(Object *object) {
    assert(object->type == CONS);

    printf("(");
    if (is_list(object)) {
        _print_seq(object);
    } else {
        object_print(object->u.cons.car);
        printf(" . ");
        object_print(object->u.cons.cdr);
    }
    printf(")");
}

/**
 * @brief Prints an object
 */
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
        case CHARACTER:
            printf("\\%c", object->u.c);
            break;
        case SYMBOL:
            printf("%s", object->u.s);
	    break;
        case CONS:
            _print_cons(object);
            break;
	case VECTOR:
            printf("[");
            _print_seq(object);
            printf("]");
            break;
        case BUILTIN:
        case SPECIAL:
        case FUNCTION:
            printf("<%s>", type_name(object->type));
            break;
        case NIL:
        case T:
            printf("%s", type_name(object->type));
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
	case VECTOR:
	    return "vector";
	default:
	    abort();
    }
}
