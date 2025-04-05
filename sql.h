#ifndef __SYSTEMR_SQL_H__
#define __SYSTEMR_SQL_H__

#include <arena.h>
#include <common.h>
#include <expressions.h>

typedef struct SelectStatement SelectStatement;

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

    Identifier *unresolved;
} SelectExpression;

typedef struct
{
    Expression *expression;

    Identifier *unresolved;
} WhereExpression;

typedef struct
{
    SelectExpression *selectList[MAX_ARRAY_SIZE];
    int selectListCount;
} SelectExpressionList;

typedef struct 
{
    const char *name;
    const char *alias;
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
    WhereExpression *whereExpression;
};

typedef struct 
{
    SelectStatement *selectStatement;
    Arena *parseArena;
    Identifier *unresolved;

    TableReference *aliasLookup[MAX_HASH_SIZE];
    bool success;
} ParsingContext;

void Finalize(ParsingContext *parsingContext, SelectStatement* selectStatement);
SelectStatement *CreateSelectStatement(ParsingContext *parsingContext, SelectExpressionList *selectExpressionList, TableReferenceList *tableReferenceList, WhereExpression *whereExpression);

SelectExpressionList *CreateSelectExpressionList(ParsingContext *parsingContext, SelectExpression *selectExpression);
SelectExpressionList *AppendSelectExpressionList(ParsingContext *parsingContext, SelectExpressionList *selectExpressionList, SelectExpression *selectExpression);
SelectExpression *CreateSelectExpression(ParsingContext *parsingContext, const char *as, Expression *expression);

TableReferenceList *CreateTableReferenceList(ParsingContext *parsingContext, TableReference *tableReference);
TableReferenceList *AppendTableReferenceList(ParsingContext *parsingContext, TableReferenceList *tableReferenceList, TableReference *tableReference);
TableReference *CreateTableReference(ParsingContext *parsingContext, const char *tableName, const char *tableAlias);

Expression *CreateStringExpression(ParsingContext *parsingContext, const char* string);
Expression *CreateNumberExpression(ParsingContext *parsingContext, long number);
Expression *CreateIdentifierExpression(ParsingContext *parsingContext, const char* qualifier, const char* name);
Expression *CreateInfixExpression(ParsingContext *parsingContext, ExpressionType expressionType, Expression *left, Expression *right);
Expression *CreateInExpression(ParsingContext *parsingContext, Expression *left, SelectStatement *selectStatement);

WhereExpression *CreateWhereExpression(ParsingContext *parsingContext, Expression *where);

#endif
