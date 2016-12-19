#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "util.h"

bool repl = false;
jmp_buf repl_jmp_buf;

void error_message(char *format, ...) {
    va_list args;

    fprintf(stderr, "Error: ");

    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);

    fprintf(stderr, "\n");
    
    if (repl)
        longjmp(repl_jmp_buf, 1);
    else
        exit(1);
}
