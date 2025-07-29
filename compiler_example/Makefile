CC = gcc
LEX = flex
YACC = bison
CFLAGS = -Wall -g -I.

all: posit_compiler posit_test posit_verifier posit8_example posit8_interpreter

posit_compiler: parser.tab.o posit.lex.o
	$(CC) -o posit_compiler parser.tab.o posit.lex.o

parser.tab.o: parser.tab.c posit.h
	$(CC) $(CFLAGS) -c parser.tab.c

posit.lex.o: posit.lex.c parser.tab.h
	$(CC) $(CFLAGS) -c posit.lex.c

parser.tab.c parser.tab.h: parser.y
	$(YACC) -d parser.y

posit.lex.c: lexer.l
	$(LEX) -o posit.lex.c lexer.l

posit_test: main.o posit.o
	$(CC) -o posit_test main.o posit.o -lm

posit_verifier: posit_verifier.o posit.o
	$(CC) -o posit_verifier posit_verifier.o posit.o -lm

posit8_example: posit8_example.o posit8.o
	$(CC) -o posit8_example posit8_example.o posit8.o -lm

posit8_interpreter: posit8_interpreter.o posit8.o
	$(CC) -o posit8_interpreter posit8_interpreter.o posit8.o -lm

main.o: main.c posit.h
	$(CC) $(CFLAGS) -c main.c

posit.o: posit.c posit.h
	$(CC) $(CFLAGS) -c posit.c

posit8.o: posit8.c posit8.h
	$(CC) $(CFLAGS) -c posit8.c

posit8_example.o: posit8_example.c posit8.h
	$(CC) $(CFLAGS) -c posit8_example.c

posit8_interpreter.o: posit8_interpreter.c posit8.h
	$(CC) $(CFLAGS) -c posit8_interpreter.c

posit_verifier.o: posit_verifier.c posit.h
	$(CC) $(CFLAGS) -c posit_verifier.c

clean:
	rm -f *.o posit_compiler posit_test posit_verifier posit8_example posit8_interpreter parser.tab.c parser.tab.h posit.lex.c