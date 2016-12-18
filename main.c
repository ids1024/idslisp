#include <stdio.h>

#include "object.h"
#include "tokenize.h"
#include "parse.h"


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
