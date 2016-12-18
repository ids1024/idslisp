#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include <stdbool.h>

typedef enum Type_ {INT, DOUBLE, STRING, LIST} Type;

struct ListNode_;

typedef struct Object_ {
    Type type;
    union {
        int d;
        double lf;
        char *s;
        struct ListNode_ *list;
    } u;
} Object;

typedef struct ListNode_ {
    struct Object_ *value;
    struct ListNode_ *next;
} ListNode;

ListNode *new_node(ListNode *next, Object *value) {
    ListNode *node = malloc(sizeof(ListNode));
    node->value = value;
    node->next = next;
    return node;
}

Object *new_int(int value) {
    Object *object = malloc(sizeof(Object));
    object->type = INT;
    object->u.d = value;
    return object;
}

Object *new_double(double value) {
    Object *object = malloc(sizeof(Object));
    object->type = DOUBLE;
    object->u.lf = value;
    return object;
}

Object *new_string(char *value) {
    Object *object = malloc(sizeof(Object));
    object->type = STRING;
    object->u.s = strdup(value);
    return object;
}

Object *new_list() {
    Object *object = malloc(sizeof(Object));
    object->type = LIST;
    object->u.list = NULL;
    return object;
}

void list_prepend(Object *object, Object *value) {
    assert(object->type == LIST);
    object->u.list = new_node(object->u.list, value);
}

Object *object_free(Object *object) {
    ListNode *node;

    switch (object->type) {
        case LIST:
            node = object->u.list;
            while (node != NULL) {
                node = object->u.list;
                free(node->value);
                free(node);
            }
            break;
        case STRING:
            free(object->u.s);
            break;
    }
    free(object);
}

void object_print(Object *object) {
    ListNode *node;

    switch (object->type) {
        case INT:
            printf("%d", object->u.d);
            break;
        case DOUBLE:
            printf("%lf", object->u.lf);
            break;
	case STRING:
            printf("\"%s\"", object->u.s);
	    break;
        case LIST:
            printf("(");
            node = object->u.list;
            while (node != NULL) {
                object_print(node->value);
                node = node->next;
                if (node != NULL) {
                    printf(" ");
                }
            }
            printf(")");
            break;
    }
}

void _add_token(char ***tokens, int *ntoks, char **curtok, int *len) {
    if (*len != 0) {
        *curtok = realloc(*curtok, *len + 1);
        (*curtok)[*len] = '\0';

        (*ntoks)++;
        *tokens = realloc(*tokens, *ntoks * sizeof(char*));
        (*tokens)[*ntoks-1] = *curtok;

        *len = 0;
        *curtok = NULL;
    }
}

char **tokenize(char *code, int *ntoks) {
    int i, len = 0;
    char *curtok = NULL;
    char **tokens = NULL;
    bool instr;

    *ntoks = 0;

    for (i=0; code[i] != '\0'; i++) {
        if (code[i] == '"') {
            _add_token(&tokens, ntoks, &curtok, &len);
            len = 1;
            curtok = strdup("\"");
            _add_token(&tokens, ntoks, &curtok, &len);
            instr = !instr;
        } else if (!instr && isspace(code[i])) {
            _add_token(&tokens, ntoks, &curtok, &len);
        } else if (!instr && (code[i] == '(' || code[i] == ')')) {
            _add_token(&tokens, ntoks, &curtok, &len);
            len = 1;
            curtok = strndup(&code[i], 1);
            _add_token(&tokens, ntoks, &curtok, &len);
        } else {
            len++;
            curtok = realloc(curtok, len);
            curtok[len-1] = code[i];
        }
    }

    _add_token(&tokens, ntoks, &curtok, &len);

    return tokens;
}

int main() {
    Object *object = new_list();
    list_prepend(object, new_int(1));
    list_prepend(object, new_double(7.2));
    list_prepend(object, new_int(3));
    list_prepend(object, new_string("test"));
    object_print(object);

    int ntoks;
    char **toks = tokenize("(print 1.2 \"test\"  (1 2 3))", &ntoks);
    printf("Tokens: %d\n", ntoks);
    for (int i = 0; i < ntoks; i++) {
        printf("%s\n", toks[i]);
    }

    return 0;
}
