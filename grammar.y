%define api.pure
%define parse.trace
%define parse.error verbose

%lex-param {void *scanner}
%parse-param {void *scanner}{ SelectStatement *selectStatement }

%{
#include <sql.h>
#include <parser.gen.h>
#include <lexer.gen.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

void yyerror(yyscan_t *locp, SelectStatement *selectStatement, const char *s);
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
%left '+' '-'
%left '*' '/'

%token AND     
%token BETWEEN 
%token CHAR    
%token SELECT  
%token FROM    
%token IN      
%token OR      
%token WHERE   

%type <SelectStatement*> select_stmt
%type <SelectExpression*> select_expr_list
%type <Expression*> select_expr
%type <TableReference*> table_refs table_ref
%type <WhereExpression*> opt_where
%type <Expression*> expr

%start select_stmt;

%%

select_stmt: 
    SELECT select_expr_list
    FROM table_refs            
    opt_where                        { 
                                       $$ = CreateSelectStatement($2, $4, $5); 
                                     }
;  

select_expr_list: 
    select_expr                      { 
                                       $$ = CreateSelectExpressionList($1); 
                                     }

  | select_expr_list ',' select_expr { 
                                       $$ = AppendSelectExpressionList($1, $3); 
                                     }
;

select_expr:
    expr                             { $$ = $1; }
  | '*'                              { $$ = NULL;}
;

table_refs:
    table_ref                        { $$ = CreateTableReferenceList($1); }
  | table_refs ',' table_ref         { $$ = AppendTableReferenceList($1, $3); } 
;

table_ref:
    "identifier"                     { $$ = CreateTableReference($1); }
;

opt_where:
                                     { $$ = NULL; }
  | WHERE expr                       { 
                                       $$ = CreateWhereExpression($2);  
                                     }
;

expr:
    "string"                         { $$ = CreateStringExpression($1);           }
  | "integer"                        { $$ = CreateNumberExpression($1);           }
  | "identifier"                     { $$ = CreateIdentifierExpression(NULL, $1); }
  | "identifier" '.' "identifier"    { $$ = CreateIdentifierExpression($1, $3);   }                   
;

expr: 
    expr '+' expr                    { $$ = CreateInfixExpression(EXPR_ADD, $1, $3); }
  | expr '-' expr                    { $$ = CreateInfixExpression(EXPR_SUB, $1, $3); }
  | expr '*' expr                    { $$ = CreateInfixExpression(EXPR_MUL, $1, $3); }
  | expr '/' expr                    { $$ = CreateInfixExpression(EXPR_DIV, $1, $3); }
  | expr EQUALITY expr               { $$ = CreateInfixExpression(EXPR_EQU, $1, $3); }
  | expr AND expr                    { $$ = CreateInfixExpression(EXPR_AND, $1, $3); }
  | expr OR expr                     { $$ = CreateInfixExpression(EXPR_OR , $1, $3); }
;

%%

void
yyerror(yyscan_t *locp, SelectStatement *selectStatement, const char *s)
{
    UNUSED(locp);
    UNUSED(selectStatement);
    UNUSED(s);
}