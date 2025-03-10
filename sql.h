#ifndef __SYSTEMR_SQL_H__
#define __SYSTEMR_SQL_H__

#include <common.h>

#define UNUSED(x) (void)(x)

typedef struct
{
    const char *qualifier;
    const char *name;
} Identifier;

typedef enum
{
    EXPR_INTEGER,
    EXPR_STRING,
    EXPR_IDENIFIER,
    EXPR_ADD,
    EXPR_SUB,
    EXPR_MUL,
    EXPR_DIV,
    EXPR_EQU,
    EXPR_AND,
    EXPR_OR,
} ExpressionType;

typedef struct
{
    ExpressionType type;
} Expression;

typedef struct infix_expr
{
    ExpressionType type;

    Expression *left;
    Expression *right;
} InfixExpression;

typedef struct term_expr
{
    ExpressionType type;

    union 
    {
        int number;
        char *string;
        Identifier *identifier;
    } value;
} TermExpression;

typedef struct {
    int i;
} SelectExpression;

typedef struct {
    int i;
} TableReference;

typedef struct {
    SelectExpression selectList[MAX_ARRAY_SIZE];
    int selectListCountount;

    TableReference tables[MAX_ARRAY_SIZE];
    int tableCount;

    Expression *whereExpression;
} SelectStatement;

typedef struct 
{
    SelectStatement selectStatment;

    Expression expressions[MAX_ARRAY_SIZE];
    int expressionCount;

    TableReference tableReferences[MAX_ARRAY_SIZE];
    int tableCount;
} ParsingContext;

SelectStatement* CreateSelectStatement(ParsingContext *parsingContext);
Expression* AppendSelectExpressionList(ParsingContext *parsingContext, Expression *expression);
TableReference* CreateTableReferenceList(ParsingContext *parsingContext, TableReference* tableReference);
TableReference* AppendTableReferenceList(ParsingContext *parsingContext, TableReference* tableReferenceList, TableReference* tableReference);
TableReference* CreateTableReference(ParsingContext *parsingContext, const char *name);

Expression* CreateStringExpression(ParsingContext *parsingContext, const char* string);
Expression* CreateNumberExpression(ParsingContext *parsingContext, long number);
Expression* CreateIdentifierExpression(ParsingContext *parsingContext, const char* qualifier, const char* name);
Expression* CreateInfixExpression(ParsingContext *parsingContext, ExpressionType expressionType, Expression *left, Expression *right);

Expression* AppendWhereExpression(ParsingContext *parsingContext, Expression *where);

#endif