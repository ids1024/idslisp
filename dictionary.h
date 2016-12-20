#ifndef DICTIONARY_H
#define DICTIONARY_H
#include "object.h"

typedef struct _DictionaryEntry_ {
    char *key;
    Object *value;
    struct _DictionaryEntry_ *next;
} _DictionaryEntry;

typedef struct Dictionary_ {
    _DictionaryEntry *first;
    struct Dictionary_ *parent;
} Dictionary;

Dictionary *dictionary_new(Dictionary *parent);
Object *dictionary_get(Dictionary *dictionary, char *key);
void dictionary_insert(Dictionary *dictionary, char *key, Object *value);
void dictionary_free(Dictionary *dictionary);
#endif
