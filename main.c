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

ListNode *new_node(ListNode *next, Object *value) {
    ListNode *node = malloc(sizeof(ListNode));
    node->value = value;
    node->next = next;
    return node;
}

Object *new_int(long int value) {
    Object *object = malloc(sizeof(Object));
    object->type = INT;
    object->u.ld = value;
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

Object *new_list(ListNode *value) {
    Object *object = malloc(sizeof(Object));
    object->type = LIST;
    object->u.list = value;
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
            printf("%d", object->u.ld);
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
    bool instr = false;

    *ntoks = 0;

    for (i=0; code[i] != '\0'; i++) {
        if (code[i] == '"') {
            _add_token(&tokens, ntoks, &curtok, &len);
            len = 1;
            curtok = malloc(1);
            curtok[0] = '"';
            _add_token(&tokens, ntoks, &curtok, &len);
            instr = !instr;
        } else if (!instr && isspace(code[i])) {
            _add_token(&tokens, ntoks, &curtok, &len);
        } else if (!instr && (code[i] == '(' || code[i] == ')')) {
            _add_token(&tokens, ntoks, &curtok, &len);
            len = 1;
            curtok = malloc(1);
            curtok[0] = code[i];
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

Object *str_to_num_object(char *text) {
    char *endptr;
    long int inum;
    double fnum;

    inum = strtol(text, &endptr, 10);
    if (*(endptr+1) == '\0')
        return new_int(inum);

    fnum = strtod(text, &endptr);
    if (*(endptr+1) == '\0')
        return new_double(fnum);

    return NULL;
}

Object *_parse_iter(char **tokens, int ntoks, int *i) {
    int j;
    Object *item;
    ListNode *list=NULL, *prev_node=NULL, *node;

    for (; *i<ntoks; (*i)++) {
        if (strcmp(tokens[*i], "\"") == 0) {
            assert((ntoks >= *i+3) && (strcmp(tokens[*i+2], "\"") == 0));
            item = new_string(tokens[*i+1]);
            (*i) += 2;
        } else if (strcmp(tokens[*i], "(") == 0) {
            (*i)++;
            item = _parse_iter(tokens, ntoks, i);
        } else if (strcmp(tokens[*i], ")") == 0) {
            return new_list(list);
        } else {
            item = str_to_num_object(tokens[*i]);
            if (item == NULL) {
                printf("Error: token '%s' not recognized.", tokens[*i]);
                exit(1);
            }
        }

        // Append to linked list
        node = new_node(NULL, item);
        if (list == NULL)
            list = node;
        if (prev_node != NULL)
            prev_node->next = node;
        prev_node = node;
    }

    printf("Error: missing ')'\n");
    exit(1);
}

Object **parse(char *code, int *nobjects) {
    int ntoks, i;
    char **tokens;
    Object *object, **objects = NULL;
    
    *nobjects = 0;
    tokens = tokenize(code, &ntoks);

    for (i=0; i<ntoks; i++) {
        if (strcmp(tokens[i], "(") == 0) {
            i++;
            object = _parse_iter(tokens, ntoks, &i);

            (*nobjects)++;
            objects = realloc(objects, *nobjects * sizeof(Object*));
            objects[*nobjects-1] = object;
        } else {
            printf("Error: '%s' not allowed here.", tokens[i]);
            exit(1);
        }
    }

    // Free the tokens array
    for (i=0; i<ntoks; i++)
        free(tokens[i]);
    free(tokens);

    return objects;
}

int main() {
    Object *object = new_list(NULL);
    list_prepend(object, new_int(1));
    list_prepend(object, new_double(7.2));
    list_prepend(object, new_int(3));
    list_prepend(object, new_string("test"));
    object_print(object);

    int ntoks;
    char **toks = tokenize("(1.2 \"test\"  (1 2 3))", &ntoks);
    printf("Tokens: %d\n", ntoks);
    for (int i = 0; i < ntoks; i++) {
        printf("%s\n", toks[i]);
    }
    int nobjects;
    Object **objects = parse("(1.2 \"test\"  (1 2 3))", &nobjects);
    for (int i = 0; i < nobjects; i++) {
        object_print(objects[i]);
        printf("\n");
    }

    return 0;
}
