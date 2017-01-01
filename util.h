/** @file */ 

#ifndef UTIL_H
#define UTIL_H
#include <stdbool.h>
#include <setjmp.h>

extern jmp_buf error_jmp_buf;

void error_message(char *format, ...) __attribute__ ((noreturn));
void _va_gc(int num, ...);

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

#define TRY_START(num, ...) \
    jmp_buf tmp_error_jmp_buf; \
    memcpy(tmp_error_jmp_buf, error_jmp_buf, sizeof(jmp_buf)); \
    if (setjmp(error_jmp_buf) != 0) { \
        _va_gc(num, __VA_ARGS__); \
        memcpy(error_jmp_buf, tmp_error_jmp_buf, sizeof(jmp_buf)); \
        longjmp(error_jmp_buf, 1); \
    }

#define TRY_END() \
    memcpy(error_jmp_buf, tmp_error_jmp_buf, sizeof(jmp_buf));

#endif
