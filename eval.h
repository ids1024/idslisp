#ifndef EVAL_H
#define EVAL_H
#include "dictionary.h"
Object *eval_arg(Dictionary *dictionary, Object *arg);
Object *eval(Dictionary *dictionary, Object *object);
#endif
