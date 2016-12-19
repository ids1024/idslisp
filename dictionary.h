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
} Dictionary;

Dictionary *dictionary_new(void);
Object *dictionary_get(Dictionary *dictionary, char *key);
void dictionary_insert(Dictionary *dictionary, char *key, Object *value);
#endif
