#ifndef __SYSTEMR_SQL_H__
#define __SYSTEMR_SQL_H__

#include <common.h>

typedef struct {
    const char *qualifier;
    const char *name;
} Identifier;

typedef enum {
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

typedef struct {
    ExpressionType type;
} Expression;

typedef struct {
    ExpressionType type;

    Expression *left;
    Expression *right;
} InfixExpression;

typedef struct {
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
    const char *name;
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
} ParsingContext;

SelectStatement* CreateSelectStatement(ParsingContext *parsingContext);
Expression* AppendSelectExpressionList(ParsingContext *parsingContext, Expression *expression);
TableReference* AppendTableReferenceList(ParsingContext *parsingContext, const char *tableName);

Expression* CreateStringExpression(ParsingContext *parsingContext, const char* string);
Expression* CreateNumberExpression(ParsingContext *parsingContext, long number);
Expression* CreateIdentifierExpression(ParsingContext *parsingContext, const char* qualifier, const char* name);
Expression* CreateInfixExpression(ParsingContext *parsingContext, ExpressionType expressionType, Expression *left, Expression *right);

Expression* AppendWhereExpression(ParsingContext *parsingContext, Expression *where);

#endif