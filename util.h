#ifndef UTIL_H
#define UTIL_H
#include <stdbool.h>
#include <setjmp.h>

extern jmp_buf error_jmp_buf;

void error_message(char *format, ...) __attribute__ ((noreturn));

#define array_append(items, nitems, value) \
    (nitems)++; \
    items = realloc(items, (nitems) * sizeof(typeof(value))); \
    (items)[nitems-1] = value;

#endif
