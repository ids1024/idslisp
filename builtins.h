#ifndef BUILTINS_H
#define BUILTINS_H
#include "object.h"
#include "dictionary.h"
Object *builtin_add(ListNode *args);
Object *builtin_minus(ListNode *args);
Object *builtin_times(ListNode *args);
Object *builtin_divide(ListNode *args);
void builtins_load(Dictionary *dictionary);
#endif
