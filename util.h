#ifndef UTIL_H
#define UTIL_H
#include <stdbool.h>
#include <setjmp.h>

extern bool repl;
extern jmp_buf repl_jmp_buf;

void error_message(char *format, ...);
#endif
