// FIXME: Faster algorithm (hash table or search tree)

#include <stdlib.h>
#include <string.h>

#include "dictionary.h"
#include "eval.h"
#include "object.h"

Dictionary *dictionary_new(Dictionary *parent) {
    Dictionary *dictionary = malloc(sizeof(Dictionary));
    *dictionary = (Dictionary){NULL, parent};
    return dictionary;
}

_DictionaryEntry *_dictionary_get(Dictionary *dictionary, char *key) {
    _DictionaryEntry *entry;

    for(entry=dictionary->first; entry!=NULL; entry=entry->next) {
        if (strcmp(entry->key, key) == 0)
            return entry;
    }
    return NULL;
}
    
Object *dictionary_get(Dictionary *dictionary, char *key) {
    _DictionaryEntry *entry = _dictionary_get(dictionary, key);
    if (entry != NULL)
        return entry->value;
    else if (dictionary->parent != NULL)
        return dictionary_get(dictionary->parent, key);
    else
        return NULL;
}

Dictionary *dictionary_top(Dictionary *dictionary) {
    while (dictionary->parent != NULL)
        dictionary = dictionary->parent;
    return dictionary;
}

void dictionary_insert(Dictionary *dictionary, char *key, Object *value) {
    _DictionaryEntry *entry;

    entry = _dictionary_get(dictionary, key);
    if (entry != NULL) {
        garbage_collect(entry->value);
        entry->value = value;
    } else {
        entry = malloc(sizeof(_DictionaryEntry));
        *entry = (_DictionaryEntry){strdup(key), value, dictionary->first};
        dictionary->first = entry;
    }
}

void dictionary_free(Dictionary *dictionary) {
    _DictionaryEntry *entry, *old_entry;
    
    entry = dictionary->first;
    while(entry != NULL) {
        free(entry->key);
        garbage_collect(entry->value);
        old_entry = entry;
        entry=entry->next;
        free(old_entry);
    }

    free(dictionary);
}
