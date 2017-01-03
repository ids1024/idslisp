#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "sequence.h"

/**
 * @brief Returns true if object is a sequence
 */
bool object_isseq(Object *object) {
    return is_list(object) || object->type == VECTOR || object->type == STRING;
}

/**
 * @brief Creates iterator from sequence
 */
Iter seq_iter(Object *object) {
    assert(object_isseq(object));
    if (is_list(object))
        return (Iter){ITERLIST, {.list = {object, true}}};
    else if (object->type == VECTOR)
        return (Iter){ITERVECTOR, {.vec = {object, 0}}};
    else if (object->type == STRING)
        return (Iter){ITERSTRING, {.vec = {object, 0}}};
    else
        abort();
}

Object *_list_first(Object *object) {
    assert(is_list(object));
    return (object == &NIL_CONST) ? NULL : object->u.cons.car;
}

Object *_list_next(Object **object) {
    if (*object == NULL)
        return NULL;

    assert(is_list(*object) && *object != &NIL_CONST);
    *object = (*object)->u.cons.cdr;

    if (!is_list(*object)) {
        Object *value = *object;
        *object = NULL;
        return value;
    } else
        return _list_first(*object);
}

/**
 * @brief Gets the next value from an iterator
 */
Object *iter_next(Iter *iter) {
    Object *value;
    char character;

    switch (iter->type) {
        case ITERLIST:
            if (iter->u.list.first)
                value = _list_first(iter->u.list.node);
            else
                value = _list_next(&iter->u.list.node);
            iter->u.list.first = false;
            break;
        case ITERVECTOR:
            if (iter->u.vec.index == iter->u.vec.object->u.vec.nitems)
                value = NULL;
            else {
                value = iter->u.vec.object->u.vec.items[iter->u.vec.index];
                iter->u.vec.index++;
            }
            break;
        case ITERSTRING:
            character = iter->u.vec.object->u.s[iter->u.vec.index];
            if (character == '\0')
                value = NULL;
            else {
                value = new_character(character);
                iter->u.vec.index++;
            }
            break;
    }

    return value;
}

/**
 * @brief Returns the length of a sequence
 */
int seq_len(Object *object) {
    int count = 0;

    if (object->type == VECTOR)
        count = object->u.vec.nitems;
    else if (object->type == STRING)
        count = strlen(object->u.s);
    else {
        Iter iter = seq_iter(object);
        Object *value;
        while ((value=iter_next(&iter)) != NULL)
            count++;
    }

    return count;
}

/**
 * @brief Returns the nth element of a sequence
 */
Object *seq_nth(Object *object, int n) {
    assert(n >= 0);

    if (object->type == VECTOR) {
        if (object->u.vec.nitems > n)
            return object->u.vec.items[n];
    } else if (object->type == STRING) {
        if (strlen(object->u.s) > n)
            return new_character(object->u.s[n]);
    } else {
        Iter iter = seq_iter(object);
        Object *value;
        int index = 0;
        while ((value=iter_next(&iter)) != NULL) {
            if (index == n)
                return value;
            index++;
        }
    }

    return NULL;
}
