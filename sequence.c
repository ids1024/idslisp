#include <stdlib.h>
#include <assert.h>

#include "sequence.h"

bool object_isseq(Object *object) {
    return is_list(object) || object->type == VECTOR;
}

Iter seq_iter(Object *object) {
    assert(object_isseq(object));
    if (is_list(object))
        return (Iter){ITERLIST, {.list = {object, true}}};
    else if (object->type == VECTOR)
        return (Iter){ITERVECTOR, {.vec = {object, 0}}};
    else
        abort();
}

Object *_list_first(Object *object) {
    assert(is_list(object));
    return (object == &NIL_CONST) ? NULL : object->u.cons.car;
}

Object *_list_next(Object **object) {
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
        return _list_first(*object);
}

Object *iter_next(Iter *iter) {
    Object *value;

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
    }

    return value;
}

int seq_len(Object *object) {
    int count = 0;
    Object *value;
    Iter iter;

    if (object->type == VECTOR)
        count = object->u.vec.nitems;
    else {
        iter = seq_iter(object);
        while ((value=iter_next(&iter)) != NULL)
            count++;
    }

    return count;
}

Object *seq_nth(Object *object, int n) {
    int index = 0;
    Object *value;
    Iter iter;

    assert(n >= 0);

    if (object->type == VECTOR) {
        if (object->u.vec.nitems > n)
            return object->u.vec.items[n];
    } else {
        iter = seq_iter(object);
        while ((value=iter_next(&iter)) != NULL) {
            if (index == n)
                return value;
            index++;
        }
    }

    return NULL;
}
