#ifndef OBJECT_H
#define OBJECT_H
#include <stdbool.h>

typedef enum Type_ {INT, DOUBLE, STRING, SYMBOL, LIST, BUILTIN, NIL, SPECIAL, FUNCTION, T} Type;

struct ListNode_;
struct Object_;
struct Dictionary_;

typedef struct Object_* (*BuiltinFunc)(struct Dictionary_*, struct ListNode_*);

typedef struct Object_ {
    Type type;
    union {
        long int ld;
        double lf;
        char *s;
        struct ListNode_ *list;
        BuiltinFunc builtin;
    } u;
    int refcount;
} Object;

typedef struct ListNode_ {
    struct Object_ *value;
    struct ListNode_ *next;
    int refcount;
} ListNode;


ListNode *new_node(ListNode *next, Object *value);
void append_node(ListNode **list, ListNode **prev, Object *value);
int list_len(ListNode *nodes) __attribute__ ((pure));
Object *list_nth(ListNode *nodes, int n) __attribute__ ((pure));
Object *new_int(long int value);
Object *new_double(double value);
Object *new_string(char *value);
Object *new_symbol(char *value);
Object *new_list(ListNode *value);
Object *new_function(ListNode *value);
Object *new_builtin(BuiltinFunc value);
Object *new_special(BuiltinFunc value);
bool object_iscallable(Object *object) __attribute__ ((pure));
bool object_issingleton(Object *object) __attribute__ ((pure));
void garbage_collect(Object *object);
void garbage_collect_list(ListNode *list);
void object_print(Object *object);

extern Object NIL_CONST;
extern Object T_CONST;

#endif
