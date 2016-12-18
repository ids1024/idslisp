all: lisp

lisp: main.o parse.o tokenize.o object.o
	gcc -g -o $@ $^

main.o: main.c parse.h object.h tokenize.h
	gcc -g -c -o $@ $<

parse.o: parse.c parse.h object.h tokenize.h
	gcc -g -c -o $@ $<

tokenize.o: tokenize.c tokenize.h
	gcc -g -c -o $@ $<

object.o: object.c object.h
	gcc -g -c -o $@ $<

clean:
	rm -f lisp .o

.PHONY: all clean
