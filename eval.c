#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "eval.h"
#include "object.h"
#include "builtins.h"
#include "util.h"


Object *eval(Dictionary *dictionary, ListNode *list) {
    char *command;
    ListNode *args;
    Object *old_val, *function;

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
            node->value = eval(dictionary, node->value->u.list);
            object_free(old_val);
        }
    }

    // Execute command
    function = dictionary_get(dictionary, command);
    if (function == NULL)
        error_message("Error: '%s' undefined.", command);
    else if (function->type == BUILTIN)
        function->u.builtin(args);
    else
        error_message("Error: '%s' not a function.", command);
}
