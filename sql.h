#ifndef __SYSTEMR_SQL_H__
#define __SYSTEMR_SQL_H__

#include <common.h>
#include <arena.h>

typedef enum
{
    ID_TABLE,
    ID_COLUMN,
} IdentifierType;

typedef struct Identifier
{
    IdentifierType type;
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

typedef struct 
{
    Expression *expression;
    const char *as;
} SelectExpression;

typedef struct 
{
    Identifier identifier;
} TableReference;

typedef struct 
{
    SelectExpression *selectList[MAX_ARRAY_SIZE];
    int selectListCount;

    TableReference tables[MAX_ARRAY_SIZE];
    int tableCount;

    Expression *whereExpression;
} SelectStatement;

typedef struct 
{
    SelectStatement selectStatment;
    Arena parseArena;
    Identifier *unresolved;
} ParsingContext;

SelectStatement *CreateSelectStatement(ParsingContext *parsingContext);
SelectExpression *AppendSelectExpressionList(ParsingContext *parsingContext, SelectExpression *selectExpression);
SelectExpression *CreateSelectExpression(ParsingContext *parsingContext, const char *as, Expression *expression);
TableReference *AppendTableReferenceList(ParsingContext *parsingContext, const char *tableName);

Expression *CreateStringExpression(ParsingContext *parsingContext, const char* string);
Expression *CreateNumberExpression(ParsingContext *parsingContext, long number);
Expression *CreateIdentifierExpression(ParsingContext *parsingContext, const char* qualifier, const char* name);
Expression *CreateInfixExpression(ParsingContext *parsingContext, ExpressionType expressionType, Expression *left, Expression *right);

Expression *AppendWhereExpression(ParsingContext *parsingContext, Expression *where);

#endif
