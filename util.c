#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "util.h"
#include "object.h"

jmp_buf error_jmp_buf; ///< Stores destination for error longjmp

/**
 * @brief Prints error message and longjmps to handler
 */
void error_message(char *format, ...) {
    va_list args;

    fprintf(stderr, "Error: ");

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");
    
    longjmp(error_jmp_buf, 1);
}

void _va_gc(int num, ...) {
    va_list args;

    va_start(args, num);
    for (int i=0; i<num; i++) {
        Object *value = va_arg(args, Object*);
        if (value != NULL)
            garbage_collect(value);
    }
    va_end(args);
}
