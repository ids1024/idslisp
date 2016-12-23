#ifndef EVAL_H
#define EVAL_H
#include "dictionary.h"
Object *eval_progn(Dictionary *dictionary, Object *nodes);
Object *call_function(Dictionary *dictionary, Object *function, Object *args);
Object *eval(Dictionary *dictionary, Object *arg);
#endif
