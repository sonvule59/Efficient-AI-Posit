CC = gcc
LEX = flex
YACC = bison
CFLAGS = -Wall -g -I.

all: posit_compiler posit_test

posit_compiler: parser.tab.o posit.lex.o
	$(CC) -o posit_compiler parser.tab.o posit.lex.o

parser.tab.o: parser.tab.c posit8.h
	$(CC) $(CFLAGS) -c parser.tab.c

posit.lex.o: posit.lex.c parser.tab.h
	$(CC) $(CFLAGS) -c posit.lex.c

parser.tab.c parser.tab.h: parser.y
	$(YACC) -d parser.y

posit.lex.c: lexer.l
	$(LEX) -o posit.lex.c lexer.l

posit_test: main.o posit8.o posit16.o softposit_fixed.o
	$(CC) -o posit_test main.o posit8.o posit16.o softposit_fixed.o -lm

posit_visualizer: posit_visualizer.o posit8.o posit16.o softposit_fixed.o
	$(CC) -o posit_visualizer posit_visualizer.o posit8.o posit16.o softposit_fixed.o -lm

main.o: main.c posit8.h
	$(CC) $(CFLAGS) -c main.c

posit8.o: posit8.c posit8.h softposit_fixed.h
	$(CC) $(CFLAGS) -c posit8.c

posit16.o: posit16.c posit16.h
	$(CC) $(CFLAGS) -c posit16.c

posit_visualizer.o: posit_visualizer.c posit8.h posit16.h
	$(CC) $(CFLAGS) -c posit_visualizer.c

softposit_fixed.o: softposit_fixed.c softposit_fixed.h
	$(CC) $(CFLAGS) -c softposit_fixed.c

clean:
	rm -f *.o posit_compiler posit_test posit_visualizer parser.tab.c parser.tab.h posit.lex.c