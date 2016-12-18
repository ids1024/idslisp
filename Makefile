all: lisp

lisp: main.o eval.c builtins.o parse.o tokenize.o object.o util.o
	gcc -g -lreadline -o $@ $^

main.o: main.c parse.h object.h tokenize.h
	gcc -g -c -o $@ $<

eval.o: eval.c eval.h builtins.h object.h
	gcc -g -c -o $@ $<

builtins.o: builtins.c builtins.h object.h
	gcc -g -c -o $@ $<

parse.o: parse.c parse.h object.h tokenize.h
	gcc -g -c -o $@ $<

tokenize.o: tokenize.c tokenize.h
	gcc -g -c -o $@ $<

object.o: object.c object.h
	gcc -g -c -o $@ $<

util.o: util.c util.h
	gcc -g -c -o $@ $<

clean:
	rm -f lisp .o

.PHONY: all clean
