#ifndef __SYSTEMR_SQL_H__
#define __SYSTEMR_SQL_H__

#include <common.h>
#include <arena.h>

typedef struct SelectStatement SelectStatement;

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

typedef struct
{
    ExpressionType type;
    Expression *left;
    SelectStatement *query;
} InQueryExpression;

typedef struct 
{
    Expression *expression;
    const char *as;
} SelectExpression;

typedef struct
{
    SelectExpression *selectList[MAX_ARRAY_SIZE];
    int selectListCount;
} SelectExpressionList;

typedef struct 
{
    const char *name;
} TableReference;

typedef struct
{
    TableReference *tableReferences[MAX_ARRAY_SIZE];
    int count;
} TableReferenceList;

struct SelectStatement
{
    SelectExpressionList *selectExpressionList;
    TableReferenceList *tableReferenceList;
    Expression *whereExpression;
};

typedef struct 
{
    SelectStatement *selectStatement;
    Arena parseArena;
    Identifier *unresolved;
} ParsingContext;

void Finalize(ParsingContext *parsingContext, SelectStatement* selectStatement);
SelectStatement *CreateSelectStatement(ParsingContext *parsingContext, SelectExpressionList *selectExpressionList, TableReferenceList *tableReferenceList, Expression *whereExpression);

SelectExpressionList *CreateSelectExpressionList(ParsingContext *parsingContext, SelectExpression *selectExpression);
SelectExpressionList *AppendSelectExpressionList(ParsingContext *parsingContext, SelectExpressionList *selectExpressionList, SelectExpression *selectExpression);
SelectExpression *CreateSelectExpression(ParsingContext *parsingContext, const char *as, Expression *expression);

TableReferenceList *CreateTableReferenceList(ParsingContext *parsingContext, const char *tableName);
TableReferenceList *AppendTableReferenceList(ParsingContext *parsingContext, TableReferenceList *tableReferenceList, const char *tableName);

Expression *CreateStringExpression(ParsingContext *parsingContext, const char* string);
Expression *CreateNumberExpression(ParsingContext *parsingContext, long number);
Expression *CreateIdentifierExpression(ParsingContext *parsingContext, const char* qualifier, const char* name);
Expression *CreateInfixExpression(ParsingContext *parsingContext, ExpressionType expressionType, Expression *left, Expression *right);
Expression *CreateInExpression(ParsingContext *parsingContext, Expression *left, SelectStatement *selectStatement);

Expression *AppendWhereExpression(ParsingContext *parsingContext, Expression *where);

#endif
