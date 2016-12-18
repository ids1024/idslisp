#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "object.h"
#include "builtins.h"
#include "util.h"


Object *eval(ListNode *list) {
    char *command;
    ListNode *args;
    Object *old_val;

    if (list == NULL)
        error_message("Cannot evaluate empty list.");

    if (list->value->type != SYMBOL)
        error_message("Cannot evaluate non-symbol.");

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
        error_message("Error: '%s' undefined.\n", command);
    }
}
