#ifndef OBJECT_H
#define OBJECT_H
typedef enum Type_ {INT, DOUBLE, STRING, SYMBOL, LIST, BUILTIN, NIL, SPECIAL} Type;

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
Object *new_int(long int value);
Object *new_double(double value);
Object *new_string(char *value);
Object *new_symbol(char *value);
Object *new_list(ListNode *value);
Object *new_builtin(BuiltinFunc value);
Object *new_special(BuiltinFunc value);
Object *new_nil(void);
void list_prepend(Object *object, Object *value);
void garbage_collect(Object *object);
void garbage_collect_list(ListNode *list);
void object_print(Object *object);

#endif
