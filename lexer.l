%option case-insensitive
%option nounput
%option noinput 
%option reentrant
%option bison-bridge
%option noyywrap

%{
#include <parser.gen.h>
#include <stdio.h>

(yyscan_t *locp, SelectStatement *selectStatement, char *s);
%}


id        [A-Za-z][A-Za-z0-9_]*
int       [0-9]+
blank     [ \t\n]
string    '(\\.|''|[^'\n])*'
operators [-+&~|^/%*(),.;!]  
equal     "="
%%

AND         { return AND;     }
BETWEEN     { return BETWEEN; }
CHAR        { return CHAR;    }
SELECT      { return SELECT;  }
FROM        { return FROM;    }
IN          { return IN;}
OR          { return OR;      }
WHERE       { return WHERE;   }

{int}       { yylval->INTNUM = atol(yytext); return INTNUM; }
 
{id}        { yylval->IDENTIFIER = strdup(yytext); return IDENTIFIER; }
{string}    { yylval->STRING = strndup(yytext + 1, strlen(yytext) - 2); return STRING; }

{operators} { return yytext[0]; }
 
{equal}     { yylval->OPERATOR = yytext; return EQUALITY; }

{blank}     ;

.           { fprintf(stdout, "lex mystery character '%c'\n", *yytext); }

%%