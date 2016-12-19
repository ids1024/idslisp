#ifndef OBJECT_H
#define OBJECT_H
typedef enum Type_ {INT, DOUBLE, STRING, SYMBOL, LIST, BUILTIN, NIL} Type;

struct ListNode_;
struct Object_;

typedef struct Object_* (*BuiltinFunc)(struct ListNode_*);

typedef struct Object_ {
    Type type;
    union {
        long int ld;
        double lf;
        char *s;
        struct ListNode_ *list;
        BuiltinFunc builtin;
    } u;
} Object;

typedef struct ListNode_ {
    struct Object_ *value;
    struct ListNode_ *next;
} ListNode;


ListNode *new_node(ListNode *next, Object *value);
Object *new_int(long int value);
Object *new_double(double value);
Object *new_string(char *value);
Object *new_symbol(char *value);
Object *new_list(ListNode *value);
Object *new_builtin(BuiltinFunc value);
Object *new_nil(void);
void list_prepend(Object *object, Object *value);
Object *object_free(Object *object);
void object_print(Object *object);
#endif
