#include <stdlib.h>
#include <stdio.h>

#include "object.h"
#include "parse.h"

char *read_to_string(FILE *file) {
    // FIXME: performance
    char *text, next_char;
    int len=0;

    text = malloc(1);
    while ((next_char = fgetc(file)) != EOF) {
        len++;
        text = realloc(text, len+1);
        text[len-1] = next_char;
    }
    text[len] = '\0';

    return text;
}

int main(int argc, char *argv[]) {
    int nobjects;
    Object **objects;
    FILE *file;
    char *text;

    if (argc < 2) {
        printf("Not enough arguments.\n");
        exit(1);
    }

    file = fopen(argv[1], "r");
    if (file == NULL) {
        perror("Error");
        exit(1);
    }

    text = read_to_string(file);

    objects = parse(text, &nobjects);
    for (int i = 0; i < nobjects; i++) {
        object_print(objects[i]);
        printf("\n");
    }

    return 0;
}
