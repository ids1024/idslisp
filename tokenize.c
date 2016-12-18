#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "tokenize.h"


void _append_token(char ***tokens, int *ntoks, char *token) {
    (*ntoks)++;
    *tokens = realloc(*tokens, *ntoks * sizeof(char*));
    (*tokens)[*ntoks-1] = token;
}

void _add_token(char ***tokens, int *ntoks, char **curtok, int *len) {
    if (*len != 0) {
        *curtok = realloc(*curtok, *len + 1);
        (*curtok)[*len] = '\0';

        _append_token(tokens, ntoks, *curtok);

        *len = 0;
        *curtok = NULL;
    }
}

char **tokenize(char *code, int *ntoks) {
    int i, len = 0;
    char *curtok = NULL;
    char **tokens = NULL;
    bool instr = false;

    *ntoks = 0;

    for (i=0; code[i] != '\0'; i++) {
        if (code[i] == '"') {
            _add_token(&tokens, ntoks, &curtok, &len);
            _append_token(&tokens, ntoks, strdup("\""));
            instr = !instr;
        } else if (!instr && isspace(code[i])) {
            _add_token(&tokens, ntoks, &curtok, &len);
        } else if (!instr && (code[i] == '(' || code[i] == ')')) {
            _add_token(&tokens, ntoks, &curtok, &len);
            _append_token(&tokens, ntoks, strndup(code+i, 1));
        } else {
            len++;
            curtok = realloc(curtok, len);
            curtok[len-1] = code[i];
        }
    }

    _add_token(&tokens, ntoks, &curtok, &len);

    return tokens;
}
