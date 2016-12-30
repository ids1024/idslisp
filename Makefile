GCCARGS = -g -Wall -Werror

all: lisp

lisp: main.o eval.c builtins.o parse.o tokenize.o sequence.o object.o util.o dictionary.o
	gcc ${GCCARGS} -lreadline -o $@ $^

main.o: main.c parse.h object.h tokenize.h eval.h util.h
	gcc ${GCCARGS} -c -o $@ $<

eval.o: eval.c eval.h builtins.h object.h
	gcc ${GCCARGS} -c -o $@ $<

builtins.o: builtins.c builtins.h object.h dictionary.h
	gcc ${GCCARGS} -c -o $@ $<

parse.o: parse.c parse.h object.h tokenize.h
	gcc ${GCCARGS} -c -o $@ $<

tokenize.o: tokenize.c tokenize.h
	gcc ${GCCARGS} -c -o $@ $<

sequence.o: sequence.c sequence.h
	gcc ${GCCARGS} -c -o $@ $<

object.o: object.c object.h
	gcc ${GCCARGS} -c -o $@ $<

util.o: util.c util.h
	gcc ${GCCARGS} -c -o $@ $<

dictionary.o: dictionary.c dictionary.h object.h
	gcc ${GCCARGS} -c -o $@ $<

clean:
	rm -f lisp *.o

.PHONY: all clean
