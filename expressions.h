#ifndef __SYSTEMR_EXPRESSION_H__
#define __SYSTEMR_EXPRESSION_H__

#include <catalog.h>

typedef struct Identifier
{
    const char *qualifier;
    const char *name;
    struct Identifier *next;
} Identifier;

typedef enum 
{
    EXPR_NUMBER,
    EXPR_STRING,
    EXPR_IDENIFIER,
    EXPR_ADD,
    EXPR_SUB,
    EXPR_MUL,
    EXPR_DIV,
    EXPR_EQU,
    EXPR_AND,
    EXPR_OR,
    EXPR_IN_QUERY,
} ExpressionType;

typedef struct 
{
    ExpressionType type;
} Expression;

typedef struct 
{
    ExpressionType type;

    Expression *left;
    Expression *right;
} InfixExpression;

typedef struct 
{
    ExpressionType type;

    union 
    {
        int number;
        const char *string;
        Identifier identifier;
    } value;
} TermExpression;

#endif
