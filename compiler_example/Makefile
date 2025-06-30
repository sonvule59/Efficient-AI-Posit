CC = gcc
LEX = flex
YACC = bison
CFLAGS = -Wall -g -I.

all: posit_compiler posit_test posit_verifier

posit_compiler: posit.tab.o posit.lex.o
	$(CC) -o posit_compiler posit.tab.o posit.lex.o -lfl

posit.tab.o: posit.tab.c posit.h
	$(CC) $(CFLAGS) -c posit.tab.c

posit.lex.o: posit.lex.c posit.tab.h
	$(CC) $(CFLAGS) -c posit.lex.c

posit.tab.c posit.tab.h: posit.y
	$(YACC) -d posit.y

posit.lex.c: posit.l
	$(LEX) -o posit.lex.c posit.l

posit_test: main.o posit.o
	$(CC) -o posit_test main.o posit.o -lm

posit_verifier: posit_verifier.o posit.o
	$(CC) -o posit_verifier posit_verifier.o posit.o -lm

main.o: main.c posit.h
	$(CC) $(CFLAGS) -c main.c

posit.o: posit.c posit.h
	$(CC) $(CFLAGS) -c posit.c

posit_verifier.o: posit_verifier.c posit.h
	$(CC) $(CFLAGS) -c posit_verifier.c

clean:
	rm -f *.o posit_compiler posit_test posit_verifier posit.tab.c posit.tab.h posit.lex.c