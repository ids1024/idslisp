/** @file */

#ifndef UTIL_H
#define UTIL_H
#include <setjmp.h>
#include <stdbool.h>

extern jmp_buf error_jmp_buf;

void error_message(char *format, ...) __attribute__((noreturn));
void _va_gc(int num, ...);

/**
 * @brief Appends to malloc'd c array
 * @param items Pointer to array (will be modified)
 * @param nitems Length of array (will be modified)
 * @param value Value to place in array
 */
#define array_append(items, nitems, value)                                     \
    ({                                                                         \
        (nitems)++;                                                            \
        items = realloc(items, (nitems) * sizeof(value));                      \
        (items)[nitems - 1] = value;                                           \
    })

/**
 * @brief Runs garbage collector on error
 * @param num Number of objects to GC
 * @param ... Objects to GC
 */
#define TRY_START(num, ...)                                                    \
    jmp_buf tmp_error_jmp_buf;                                                 \
    memcpy(tmp_error_jmp_buf, error_jmp_buf, sizeof(jmp_buf));                 \
    if (setjmp(error_jmp_buf) != 0) {                                          \
        _va_gc(num, __VA_ARGS__);                                              \
        TRY_END();                                                             \
        longjmp(error_jmp_buf, 1);                                             \
    }

/**
 * @brief Cleans up changes made by @ref TRY_START
 */
#define TRY_END() memcpy(error_jmp_buf, tmp_error_jmp_buf, sizeof(jmp_buf));

#endif
