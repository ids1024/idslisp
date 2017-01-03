#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "tokenize.h"
#include "util.h"


void _add_token(char ***tokens, int *ntoks, char **curtok, int *len) {
    if (*len != 0) {
        array_append(*curtok, *len, '\0');
        array_append(*tokens, *ntoks, *curtok);
        *len = 0;
        *curtok = NULL;
    }
}

char **tokenize(char *code, int *ntoks) {
    int len = 0;
    char *curtok = NULL;
    char **tokens = NULL;
    bool instr = false;

    *ntoks = 0;

    for (int i=0; code[i] != '\0'; i++) {
        if (code[i] == '"') {
            int len=0;
            _add_token(&tokens, ntoks, &curtok, &len);
            array_append(tokens, *ntoks, strdup("\""));
            instr = !instr;
        } else if (!instr && isspace(code[i])) {
            _add_token(&tokens, ntoks, &curtok, &len);
        } else if (!instr && (code[i] == '(' || code[i] == ')' ||
                              code[i] == '[' || code[i] == ']' ||
                              code[i] == '\'')) {
            _add_token(&tokens, ntoks, &curtok, &len);
            array_append(tokens, *ntoks, strndup(code+i, 1));
        } else {
            array_append(curtok, len, code[i]);
        }
    }

    _add_token(&tokens, ntoks, &curtok, &len);

    return tokens;
}
