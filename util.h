#ifndef UTIL_H
#define UTIL_H
#include <stdbool.h>
#include <setjmp.h>

extern jmp_buf error_jmp_buf;

void error_message(char *format, ...);
#endif
