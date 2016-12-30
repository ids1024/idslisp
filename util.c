#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "util.h"

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
