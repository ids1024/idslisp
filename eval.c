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
    function = dictionary_get(dictionary, command);

    if (function->type == BUILTIN) {
        // Recursively evaluate arguments
        for (ListNode *node=args; node!=NULL; node=node->next) {
            if (node->value->type == LIST) {
                // Replace list with what it evluates to
                old_val = node->value;
                node->value = eval(dictionary, node->value->u.list);
                //object_free(old_val) XXX;
            } else if (node->value->type == SYMBOL) {
                old_val = node->value;
                node->value = dictionary_get(dictionary, old_val->u.s);
                if (node->value == NULL)
                    error_message("'%s' undefined.", command);
                object_free(old_val);
            }
        }
    }

    // Execute command
    if (function == NULL)
        error_message("'%s' undefined.", command);
    else if (function->type == BUILTIN || function->type == SPECIAL)
        function->u.builtin(dictionary, args);
    else
        error_message("'%s' not a function.", command);
}
