#ifndef OBJECT_H
#define OBJECT_H
#include <stdbool.h>

typedef enum Type_ {INT, DOUBLE, STRING, SYMBOL, CONS, BUILTIN, NIL, SPECIAL, FUNCTION, T} Type;

struct Object_;
struct Dictionary_;

typedef struct Object_* (*BuiltinFunc)(struct Dictionary_*, struct Object_*);

typedef struct Object_ {
    Type type;
    union {
        long int ld;
        double lf;
        char *s;
        BuiltinFunc builtin;
        struct Object_ *obj;
        struct {
            struct Object_ *car;
            struct Object_ *cdr;
        } cons;
    } u;
    int refcount;
} Object;

#define ref(obj) ({ \
    Object *_obj = obj; \
    if (_obj != NULL) \
        _obj->refcount++; \
    _obj; \
})


void append_node(Object **list, Object **prev, Object *value);
bool is_list(Object *object);
Object *list_first(Object *object) __attribute__ ((pure));
Object *list_next(Object **object);
int list_len(Object *nodes) __attribute__ ((pure));
Object *list_nth(Object *nodes, int n) __attribute__ ((pure));
Object *new_int(long int value);
Object *new_double(double value);
Object *new_string(char *value);
Object *new_symbol(char *value);
Object *new_cons(Object *car, Object *cdr);
Object *new_function(Object *value);
Object *new_builtin(BuiltinFunc value);
Object *new_special(BuiltinFunc value);
Object *from_bool(bool value);
bool to_bool(Object *value);
bool object_iscallable(Object *object) __attribute__ ((pure));
bool object_issingleton(Object *object) __attribute__ ((pure));
void garbage_collect(Object *object);
void object_print(Object *object);

extern Object NIL_CONST;
extern Object T_CONST;

#endif
