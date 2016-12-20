#ifndef EVAL_H
#define EVAL_H
#include "dictionary.h"
Object *call_function(Dictionary *dictionary, Object *function, ListNode *args);
Object *eval_arg(Dictionary *dictionary, Object *arg);
Object *eval(Dictionary *dictionary, Object *object);
#endif
