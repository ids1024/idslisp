#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "object.h"
#include "builtins.h"


Object *eval(ListNode *list) {
    char *command;
    ListNode *args;
    Object *old_val;

    if (list == NULL) {
        printf("Error: Cannot evaluate empty list.\n");
        exit(1);
    }

    if (list->value->type != SYMBOL) {
        printf("Error: Cannot evaluate non-symbol.\n");
        object_print(list->value);
        printf("\n");
        exit(1);
    }

    command = list->value->u.s;
    args = list->next;

    // Recursively evaluate arguments
    for (ListNode *node=args; node!=NULL; node=node->next) {
        if (node->value->type == LIST) {
            // Replace list with what it evluates to
            old_val = node->value;
            node->value = eval(node->value->u.list);
            object_free(old_val);
        }
    }

    // Execute command
    if (strcmp(command, "+") == 0) {
        return builtin_add(args);
    } else if (strcmp(command, "-") == 0) {
        return builtin_minus(args);
    } else if (strcmp(command, "*") == 0) {
        return builtin_times(args);
    } else if (strcmp(command, "/") == 0) {
        return builtin_divide(args);
    } else {
        printf("Error: '%s' undefined.\n");
        exit(1);
    }
}
