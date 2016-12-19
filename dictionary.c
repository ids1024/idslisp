// FIXME: Faster algorithm (hash table or search tree)

#include <stdlib.h>
#include <string.h>

#include "dictionary.h"

Dictionary *dictionary_new(void) {
    Dictionary *dictionary = malloc(sizeof(Dictionary));
    dictionary->first = NULL;
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
    if (entry == NULL)
        return NULL;
    else
        return entry->value;
}

void dictionary_insert(Dictionary *dictionary, char *key, Object *value) {
    _DictionaryEntry *entry;

    entry = _dictionary_get(dictionary, key);
    if (entry != NULL) {
        // FIXME free memory? (TODO: implement garbage collector)
        entry->value = value;
    } else {
        entry = malloc(sizeof(Dictionary));
        entry->key = strdup(key);
        entry->value = value;
        entry->next = dictionary->first;
        dictionary->first = entry;
    }
}