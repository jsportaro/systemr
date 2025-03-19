CFLAGS = -I. -Wall -Wextra -g3
LEX = flex
YACC = bison

.PHONY: clean

systemr: arena.c main.c sql.c parser.c sql.c parser.gen.c lexer.gen.c
	gcc $(CFLAGS) -o systemr arena.c main.c sql.c parser.c parser.gen.c lexer.gen.c

lexer.gen.c lexer.gen.h: lexer.l
	${LEX} --header-file=lexer.gen.h --outfile=lexer.gen.c lexer.l

parser.gen.c parser.gen.h: grammar.y
	${YACC} --defines=parser.gen.h --output=parser.gen.c grammar.y

clean:
	rm -f *.o *.gen.* systemr