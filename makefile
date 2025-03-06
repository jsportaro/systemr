CFLAGS = -I. -Wall -Wextra -g3
LEX = flex
YACC = bison

.PHONY: clean

parser.gen.o: parser.gen.c lexer.gen.h sql.h

lexer.gen.c lexer.gen.h: lexer.l
	flex --header-file=lexer.gen.h --outfile=lexer.gen.c lexer.l

parser.gen.c parser.gen.h: grammar.y
	${YACC} --header=parser.gen.h --output=parser.gen.c grammar.y

clean:
	rm -f *.o *.gen.c *.gen.h