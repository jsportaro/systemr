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
   LogicalProjection *first;
   LogicalProjection *last;
} LogicalProjections;


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
    LogicalScan *scans;
    LogicalSelection *selection;
    Arena *parseArena;
    Identifier *unresolved;
    bool allAttributes;
    LogicalScan *aliasLookup[MAX_HASH_SIZE];
} ParsingContext;

typedef struct
{
    bool success;
    bool allAttributes;

    Plan *plan;
    LogicalScan *scans;
} ParsingResult;

void Finalize(ParsingContext *parsingContext, Plan* plan);

Plan *CreatePlan(ParsingContext *parsingContext, LogicalProjections *projections, PlanNode *tables, LogicalSelection *selection);

LogicalProjections *BeginProjections(ParsingContext *parsingContext, LogicalProjection *first);
LogicalProjections *LinkProjection(LogicalProjections *projections, LogicalProjection *next);
LogicalProjection *CreateProjection(ParsingContext *parsingContext, const char *as, Expression *expression);
LogicalProjection *CreateProjectionAll(ParsingContext *parsingContext);

PlanNode *ScanToPlan(LogicalScan *scan);
PlanNode *CreateJoin(ParsingContext *parsingContext, PlanNode *left, LogicalScan *right);       

LogicalScan *CreateScan(ParsingContext *parsingContext, const char *tableName, const char *tableAlias);

Expression *CreateStringExpression(ParsingContext *parsingContext, const char* string);
Expression *CreateNumberExpression(ParsingContext *parsingContext, long number);
Expression *CreateIdentifierExpression(ParsingContext *parsingContext, const char* qualifier, const char* name);
Expression *CreateInfixExpression(ParsingContext *parsingContext, ExpressionType expressionType, Expression *left, Expression *right);
Expression *CreateInExpression(ParsingContext *parsingContext, Expression *left, Plan *plan);

LogicalSelection *CreateSelection(ParsingContext *parsingContext, Expression *where);

#endif
