Assembler: Assembler.o Checkers.o Insertion.o
	gcc -g -pedantic -Wall -ansi Assembler.o Checkers.o Insertion.o -o Assembler -lm

Assembler.o: Assembler.c 
	gcc -c -pedantic -Wall -ansi Assembler.c -o Assembler.o -lm

Checkers.o: Checkers.c
	gcc -c -pedantic -Wall -ansi Checkers.c -o Checkers.o -lm

Insertion.o: Insertion.c
	gcc -c -pedantic -Wall -ansi Insertion.c -o Insertion.o -lm

