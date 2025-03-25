%define api.pure
%define parse.trace
%define parse.error verbose

%lex-param {void *scanner}
%parse-param {void *scanner}{ ParsingContext *parsingContext }

%{
#include <sql.h>
#include <parser.gen.h>
#include <lexer.gen.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

void yyerror(yyscan_t *locp, ParsingContext *parsingContext, const char *s);
%}

%code requires
{
#include <sql.h>
}

%define api.value.type union /* Generate YYSTYPE from these types:  */
%token <long>           INTNUM     "integer"
%token <const char *>   STRING     "string"
%token <const char *>   IDENTIFIER "identifier"
%token <const char *>   OPERATOR   "operator"

%left AND
%left OR
%left EQUALITY
%nonassoc IN
%left '+' '-'
%left '*' '/'

%token AND
%token AS
%token BETWEEN 
%token CHAR    
%token SELECT  
%token FROM    
%token IN      
%token OR      
%token WHERE   

%type <SelectStatement *> select_stmt
%type <SelectExpressionList *> select_expr_list
%type <SelectExpression *> select_expr
%type <TableReferenceList *> table_refs 
%type <const char *> table_ref
%type <Expression *> opt_where
%type <Expression *> expr

%start sql_start;

%%

sql_start:
  select_stmt                       {
                                      Finalize(parsingContext, $1);
                                    }

select_stmt: 
    SELECT select_expr_list
    FROM table_refs            
    opt_where                        { 
                                       $$ = CreateSelectStatement(parsingContext, $2, $4, $5);
                                     }
;  

select_expr_list: 
    select_expr                      { 
                                       $$ = CreateSelectExpressionList(parsingContext, $1); 
                                     }

  | select_expr_list ',' select_expr { 
                                       $$ = AppendSelectExpressionList(parsingContext, $1, $3); 
                                     }
;

select_expr:
    expr                             { $$ = CreateSelectExpression(parsingContext, NULL, $1); }
  | expr AS "identifier"             { $$ = CreateSelectExpression(parsingContext, $3, $1);   }
  | '*'                              { $$ = NULL;}
;

table_refs:
    table_ref                        { $$ = CreateTableReferenceList(parsingContext, $1);     }
  | table_refs ',' table_ref         { $$ = AppendTableReferenceList(parsingContext, $1, $3); } 
;

table_ref:
    "identifier"                     { $$ = $1; }
;

opt_where:
                                     { $$ = NULL; }
  | WHERE expr                       { 
                                       $$ = AppendWhereExpression(parsingContext, $2);  
                                     }
;

expr:
    "string"                         { $$ = CreateStringExpression(parsingContext, $1);           }
  | "integer"                        { $$ = CreateNumberExpression(parsingContext, $1);           }
  | "identifier"                     { $$ = CreateIdentifierExpression(parsingContext, NULL, $1); }
  | "identifier" '.' "identifier"    { $$ = CreateIdentifierExpression(parsingContext, $1, $3);   }                   
;

expr: 
    expr '+' expr                    { $$ = CreateInfixExpression(parsingContext, EXPR_ADD, $1, $3); }
  | expr '-' expr                    { $$ = CreateInfixExpression(parsingContext, EXPR_SUB, $1, $3); }
  | expr '*' expr                    { $$ = CreateInfixExpression(parsingContext, EXPR_MUL, $1, $3); }
  | expr '/' expr                    { $$ = CreateInfixExpression(parsingContext, EXPR_DIV, $1, $3); }
  | expr EQUALITY expr               { $$ = CreateInfixExpression(parsingContext, EXPR_EQU, $1, $3); }
  | expr AND expr                    { $$ = CreateInfixExpression(parsingContext, EXPR_AND, $1, $3); }
  | expr OR expr                     { $$ = CreateInfixExpression(parsingContext, EXPR_OR , $1, $3); }
;

expr:
    expr IN '(' select_stmt ')'      { $$ = CreateInExpression(parsingContext, $1, $4); }
;

%%

void
yyerror(yyscan_t *locp, ParsingContext *parsingContext, const char *s)
{
    UNUSED(locp);
    UNUSED(parsingContext);
    UNUSED(s);
}
