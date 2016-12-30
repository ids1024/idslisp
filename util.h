/** @file */ 

#ifndef UTIL_H
#define UTIL_H
#include <stdbool.h>
#include <setjmp.h>

extern jmp_buf error_jmp_buf;

void error_message(char *format, ...) __attribute__ ((noreturn));

/**
 * @brief Appends to malloc'd c array
 * @param items Pointer to array (will be modified)
 * @param nitems Length of array (will be modified)
 * @param value Value to place in array
 */
#define array_append(items, nitems, value) ({ \
    (nitems)++; \
    items = realloc(items, (nitems) * sizeof(value)); \
    (items)[nitems-1] = value; \
})

#endif
