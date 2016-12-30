#ifndef SEQUENCE_H
#define SEQUENCE_H

#include "object.h"


typedef enum IterType_ {ITERLIST, ITERVECTOR, ITERSTRING} IterType;

typedef struct Iter_ {
    IterType type;
    union {
        struct {
            Object *node;
            bool first;
        } list;
        struct {
            Object *object;
            int index;
        } vec;
    } u;
} Iter;

bool object_isseq(Object *object) __attribute__ ((pure));
Iter seq_iter(Object *object);
Object *iter_next(Iter *iter);
int seq_len(Object *object) __attribute__ ((pure));
Object *seq_nth(Object *object, int n) __attribute__ ((pure));

#endif
