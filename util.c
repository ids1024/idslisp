#include <stdio.h>
#include <stdarg.h>

#include "util.h"

jmp_buf error_jmp_buf;

void error_message(char *format, ...) {
    va_list args;

    fprintf(stderr, "Error: ");

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");
    
    longjmp(error_jmp_buf, 1);
}
