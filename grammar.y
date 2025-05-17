%define api.pure
%define parse.trace
%define parse.error verbose

%lex-param {void *scanner}
%parse-param {void *scanner}{ ParsingContext *parsingContext }

%{
#include <sql.h>
#include <plan.h>
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

%type <Plan *> select_stmt
%type <Projections *> select_expr_list
%type <Projection *> select_expr
%type <PlanNode *> table_refs 
%type <Scan *> table_ref
%type <Selection *> opt_where
%type <Expression *> expr
%type <const char *> table_alias

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
                                       $$ = CreatePlan(parsingContext, $2, $4, $5);
                                     }
;  

select_expr_list: 
    select_expr                      { 
                                       $$ = BeginProjections(parsingContext, $1); 
                                     }

  | select_expr_list ',' select_expr { 
                                       $$ = LinkProjection($1, $3); 
                                     }
;

select_expr:
    expr                             { $$ = CreateProjection(parsingContext, NULL, $1); }
  | expr AS "identifier"             { $$ = CreateProjection(parsingContext, $3, $1);   }
  | '*'                              { $$ = CreateProjectionAll(parsingContext);        }
;
  
table_refs:
    table_ref                        { $$ = ScanToPlan($1);     }
  | table_refs ',' table_ref         { $$ = CreateJoin(parsingContext, $1, $3); } 
;
  
table_ref:
    "identifier"                     { $$ = CreateScan(parsingContext, $1, NULL); }
  | "identifier" table_alias         { $$ = CreateScan(parsingContext, $1, $2); }

;
    
table_alias:
    AS "identifier"                  { $$ = $2; }
  | "identifier"                     { $$ = $1; }
;

opt_where:
                                     { $$ = NULL; }
  | WHERE expr                       { 
                                       $$ = CreateSelection(parsingContext, $2);  
                                     }
;

expr:
    "string"                         { $$ = CreateStringExpression(parsingContext, $1);           }
  | "integer"                        { $$ = CreateNumberExpression(parsingContext, $1);           }
  | "identifier"                     { $$ = CreateIdentifierExpression(parsingContext, NULL, $1); }
  | "identifier" '.' "identifier"    { $$ = CreateIdentifierExpression(parsingContext, $1, $3);   }      
  | '(' expr ')'                     { $$ = CreateExpressionGroup(parsingContext, $2);            }
;

expr: 
    expr '+' expr                    { $$ = CreateInfixExpression(parsingContext, EXPR_ADD, $1, $3); }
  | expr '-' expr                    { $$ = CreateInfixExpression(parsingContext, EXPR_SUB, $1, $3); }
  | expr '*' expr                    { $$ = CreateInfixExpression(parsingContext, EXPR_MUL, $1, $3); }
  | expr '/' expr                    { $$ = CreateInfixExpression(parsingContext, EXPR_DIV, $1, $3); }
  | expr EQUALITY expr               { $$ = CreateInfixExpression(parsingContext, EXPR_EQU, $1, $3); }
  | expr AND expr                    { $$ = CreateAndExpression(parsingContext, $1, $3); }
  | expr OR expr                     { $$ = CreateOrExpression(parsingContext, $1, $3); }
;

expr:
    expr IN '(' select_stmt ')'      { $$ = CreateInExpression(parsingContext, $1, $4); }
;

%%

void yyerror(yyscan_t *locp, ParsingContext *parsingContext, const char *s)
{
    UNUSED(locp);
    UNUSED(parsingContext);

    printf("Error %s\n", s);
}
