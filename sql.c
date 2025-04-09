#include <arena.h>
#include <sql.h>

#include <string.h>


void Finalize(ParsingContext *parsingContext, Plan* plan)
{
    parsingContext->plan = plan;
}

Plan *CreatePlan(ParsingContext *parsingContext, LogicalProjections *projections, PlanNode *tables, LogicalSelection *selection)
{
    Plan *plan = NEW(parsingContext->parseArena, Plan);

    plan->root = (PlanNode *)projections->first;
    projections->last->child = (PlanNode *)selection;

    if (selection != NULL)
    {
        selection->child = tables;
    }
    else
    {
        projections->last->child = tables;
    }

    return plan;
}

LogicalProjections *BeginProjections(ParsingContext *parsingContext, LogicalProjection *first)
{
    LogicalProjections *projections = NEW(parsingContext->parseArena, LogicalProjections);

    projections->first = first;
    projections->last = first;

    return projections;
}

LogicalProjections *LinkProjection(LogicalProjections *projections, LogicalProjection *next)
{
    projections->last->child = (PlanNode *)next;
    projections->last = next;
    return projections;
}

LogicalProjection *CreateProjectionAll(ParsingContext *parsingContext)
{
    LogicalProjection *projection = NEW(parsingContext->parseArena, LogicalProjection);
    
    parsingContext->allAttributes = true;
    projection->type = LPLAN_PROJECT_ALL;
    
    return projection;
}

LogicalProjection *CreateProjection(ParsingContext *parsingContext, const char *as, Expression *expression)
{
    UNUSED(as);

    LogicalProjection *projection = NEW(parsingContext->parseArena, LogicalProjection);

    projection->projected = expression;
    projection->type = LPLAN_PROJECT;
    projection->unresolved = parsingContext->unresolved;
    
    parsingContext->unresolved = NULL;

    return projection;
}

PlanNode *ScanToPlan(LogicalScan *scan)
{
    return (PlanNode *)scan;
}

PlanNode *CreateJoin(ParsingContext *parsingContext, PlanNode *left, LogicalScan *right)
{
    LogicalJoin *join = NEW(parsingContext->parseArena, LogicalJoin);

    join->left = left;
    join->right = (PlanNode *)right;
    join->type = LPLAN_JOIN;
    
    return (PlanNode *)join;
}

LogicalScan *CreateScan(ParsingContext *parsingContext, const char *tableName, const char *tableAlias)
{
    LogicalScan *scan = NEW(parsingContext->parseArena, LogicalScan);

    scan->name = tableName;
    scan->type = LPLAN_SCAN;
    scan->next = parsingContext->scans;
    parsingContext->scans = scan;

    //  Rewrite to force all table references to have an alias
    //  Helps reduce string compares later when matching alias to tables
    //  For example:
    //      SELECT table1.col1 From table1
    scan->alias = tableAlias == NULL ? tableName : tableAlias;

    return scan;
}

Expression *CreateStringExpression(ParsingContext *parsingContext, const char* string)
{
    TermExpression *expression = NEW(parsingContext->parseArena, TermExpression);
    
    expression->value.string = string;
    expression->type = EXPR_STRING;

    return (Expression *)expression;
}

Expression *CreateNumberExpression(ParsingContext *parsingContext, long number)
{
    TermExpression *expression = NEW(parsingContext->parseArena, TermExpression);
    
    expression->value.number = number;
    expression->type = EXPR_NUMBER;

    return (Expression *)expression;
}

Expression *CreateIdentifierExpression(ParsingContext *parsingContext, const char* qualifier, const char* name)
{
    TermExpression *expression = NEW(parsingContext->parseArena, TermExpression);

    expression->value.identifier.qualifier = qualifier;
    expression->value.identifier.name = name;
    expression->type = EXPR_IDENIFIER;

    expression->value.identifier.next = parsingContext->unresolved;
    parsingContext->unresolved = &expression->value.identifier;

    return (Expression *)expression;
}

Expression *CreateInfixExpression(ParsingContext *parsingContext, ExpressionType expressionType, Expression *left, Expression *right)
{
    InfixExpression *expression = NEW(parsingContext->parseArena, InfixExpression);
    
    expression->left = left;
    expression->right = right;
    expression->type = expressionType;

    return (Expression *)expression;
}

Expression *CreateInExpression(ParsingContext *parsingContext, Expression *left, Plan *plan)
{
    InQueryExpression *expression = NEW(parsingContext->parseArena, InQueryExpression);

    expression->left = left;
    expression->plan = plan;
    expression->type = EXPR_IN_QUERY;

    return (Expression *)expression;
}

LogicalSelection *CreateSelection(ParsingContext *parsingContext, Expression *where)
{
    LogicalSelection *selection = NEW(parsingContext->parseArena, LogicalSelection);

    selection->condition = where;
    selection->unresolved = parsingContext->unresolved;
    selection->type = LPLAN_SELECT;

    parsingContext->unresolved = NULL;

    return selection;
}
