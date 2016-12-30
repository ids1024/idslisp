/** @file */ 

#ifndef OBJECT_H
#define OBJECT_H
#include <stdbool.h>

// LIST and PAIR are used as return values of object_type for CONS
typedef enum Type_ {INT, DOUBLE, STRING, SYMBOL, CONS, BUILTIN, NIL, SPECIAL, FUNCTION, T, LIST, PAIR, VECTOR, CHARACTER} Type;

struct Object_;
struct Dictionary_;

typedef struct Object_* (*BuiltinFunc)(struct Dictionary_*, struct Object_*);

/**
 * @brief Lisp object
 */
typedef struct Object_ {
    Type type; ///< Type of object
    union {
        long int ld; ///< Integer
        double lf; ///< Double
        char *s;  ///< String
        char c; ///< Character
        BuiltinFunc builtin; ///< Builtin function
        struct Object_ *obj; ///< Object (used for user function)
        struct {
            struct Object_ *car;
            struct Object_ *cdr;
        } cons; ///< Cons cell
        struct {
            struct Object_ **items;
            int nitems;
        } vec; ///< Vector
    } u; ///< Value union
    int refcount; ///< Reference count (for garbage collection)
} Object;

/**
 * @brief Passes object through while incrementing reference count
 */
#define ref(obj) ({ \
    Object *_obj = obj; \
    if (_obj != NULL) \
        _obj->refcount++; \
    _obj; \
})


void append_node(Object **list, Object **prev, Object *value);
bool is_list(Object *object);
Object *new_int(long int value);
Object *new_double(double value);
Object *new_string(char *value);
Object *new_character(char character);
Object *new_symbol(char *value);
Object *new_cons(Object *car, Object *cdr);
Object *new_vector(Object **items, int nitems);
Object *new_function(Object *value);
Object *new_builtin(BuiltinFunc value);
Object *new_special(BuiltinFunc value);
Object *from_bool(bool value);
bool to_bool(Object *value);
bool object_iscallable(Object *object) __attribute__ ((pure));
bool object_issingleton(Object *object) __attribute__ ((pure));
void garbage_collect(Object *object);
void object_print(Object *object);
Type object_type(Object *object) __attribute__ ((pure));
char *type_name(Type type) __attribute__ ((pure));

extern Object NIL_CONST;
extern Object T_CONST;

#endif
