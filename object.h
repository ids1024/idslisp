#ifndef OBJECT_H
#define OBJECT_H
typedef enum Type_ {INT, DOUBLE, STRING, LIST} Type;

struct ListNode_;

typedef struct Object_ {
    Type type;
    union {
        long int ld;
        double lf;
        char *s;
        struct ListNode_ *list;
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
Object *new_list(ListNode *value);
void list_prepend(Object *object, Object *value);
Object *object_free(Object *object);
void object_print(Object *object);
#endif
