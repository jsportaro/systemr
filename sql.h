#ifndef __SYSTEMR_SQL_H__
#define __SYSTEMR_SQL_H__

#include <arena.h>
#include <common.h>
#include <plan.h>

typedef struct SelectStatement SelectStatement;

typedef struct
{
    ExpressionType type;
    Expression *left;
    Plan *plan;
} InQueryExpression;

typedef struct 
{
    Expression *expression;
    const char *as;

    Identifier *unresolved;
} SelectExpression;

typedef struct 
{
    String name;
    String alias;
} TableReference;

typedef struct
{
    TableReference *tableReferences[MAX_ARRAY_SIZE];
    int count;
} TableReferenceList;

// There's a bug here - ParsingContext has to be scoped to a query
// NOT global to the entire query
// Subqueries will break this
// SELECT 
//   p.name, 
//   age, 
//   id 
// FROM 
//   person p,
//   place
//   thing 
// WHERE
//   p.name IN
//   (             <--  Need to introduce a new ParsingContext here - probably need to stack them
//      SELECT 
//        name
//      FROM
//        person
//   );
typedef struct 
{
    bool success;
    Plan *plan;
    Scan *scans;
    ScanList scanList;
    Selection *selection;
    Arena *parseArena;
    Referenced referenced;

    bool allAttributes;

    int orCount;
    int andCount;
} ParsingContext;

typedef struct
{
    bool success;
    bool allAttributes;

    Plan *plan;
    Scan *scans;
} ParsingResult;

void Finalize(ParsingContext *parsingContext, Plan* plan);

Plan *CreatePlan(ParsingContext *parsingContext, Projections *projections, PlanNode *tables, Selection *selection);

Projections *BeginProjections(ParsingContext *parsingContext, Projection *first);
Projections *LinkProjection(Projections *projections, Projection *next);
Projection *CreateProjection(ParsingContext *parsingContext, const char *as, Expression *expression);
Projection *CreateProjectionAll(ParsingContext *parsingContext);

PlanNode *ScanToPlan(Scan *scan);
PlanNode *CreateJoin(ParsingContext *parsingContext, PlanNode *left, Scan *right);       

Scan *CreateScan(ParsingContext *parsingContext, const char *tableName, const char *tableAlias);

Expression *CreateExpressionGroup(ParsingContext *parsingContext, Expression *expression);
Expression *CreateStringExpression(ParsingContext *parsingContext, const char* string);
Expression *CreateNumberExpression(ParsingContext *parsingContext, long number);
Expression *CreateIdentifierExpression(ParsingContext *parsingContext, const char* qualifier, const char* name);
Expression *CreateInfixExpression(ParsingContext *parsingContext, ExpressionType expressionType, Expression *left, Expression *right);
Expression *CreateAndExpression(ParsingContext *parsingContext, Expression *left, Expression *right);
Expression *CreateOrExpression(ParsingContext *parsingContext, Expression *left, Expression *right);
Expression *CreateInExpression(ParsingContext *parsingContext, Expression *left, Plan *plan);

Selection *CreateSelection(ParsingContext *parsingContext, Expression *where);

#endif
