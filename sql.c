#include <arena.h>
#include <sql.h>
#include <slice.h>
#include <rstrings.h>

#include <string.h>


void Finalize(ParsingContext *parsingContext, Plan* plan)
{
    parsingContext->plan = plan;
}

Plan *CreatePlan(ParsingContext *parsingContext, Projections *projections, PlanNode *tables, Selection *selection)
{
    UNUSED(tables); //  I think I can get rid of tables entirely

    
    Plan *plan = NEW(parsingContext->parseArena, Plan);

    plan->projections = projections;
    plan->scanList = parsingContext->scanList;
    plan->selection = selection;
    plan->referenced = parsingContext->referenced;
    plan->pushDown = !parsingContext->allAttributes;

    parsingContext->scans = NULL;
    parsingContext->selection = NULL;
    
    return plan;
}

Projections *BeginProjections(ParsingContext *parsingContext, Projection *first)
{
    Projections *projections = NEW(parsingContext->parseArena, Projections);

    projections->first = first;
    projections->last = first;

    return projections;
}

Projections *LinkProjection(Projections *projections, Projection *next)
{
    projections->last->child = (PlanNode *)next;
    projections->last = next;
    return projections;
}

Projection *CreateProjectionAll(ParsingContext *parsingContext)
{
    Projection *projection = NEW(parsingContext->parseArena, Projection);
    
    parsingContext->allAttributes = true;
    projection->type = LPLAN_PROJECT_ALL;
    
    return projection;
}

Projection *CreateProjection(ParsingContext *parsingContext, const char *as, Expression *expression)
{
    UNUSED(as);

    Projection *projection = NEW(parsingContext->parseArena, Projection);

    projection->projected = expression;
    projection->type = LPLAN_PROJECT;

    return projection;
}

PlanNode *ScanToPlan(Scan *scan)
{
    return (PlanNode *)scan;
}

PlanNode *CreateJoin(ParsingContext *parsingContext, PlanNode *left, Scan *right)
{
    Join *join = NEW(parsingContext->parseArena, Join);

    join->left = left;
    join->right = (PlanNode *)right;
    join->type = LPLAN_JOIN;
    
    return (PlanNode *)join;
}

Scan *CreateScan(ParsingContext *parsingContext, const char *tableName, const char *tableAlias)
{
    Scan *scan = NEW(parsingContext->parseArena, Scan);

    scan->name = S(tableName);
    scan->type = LPLAN_SCAN;
    parsingContext->scans = scan;
    scan->filter = NULL;
    //  Rewrite to force all table references to have an alias
    //  Helps reduce string compares later when matching alias to tables
    //  For example:
    //      SELECT table1.col1 From table1
    scan->alias = tableAlias == NULL ? scan->name : S(tableAlias);

    *Push(&parsingContext->scanList, parsingContext->parseArena) = scan;

    return scan;
}

Expression *CreateExpressionGroup(ParsingContext *parsingContext, Expression *expression)
{
    ExpressionGroup *group = NEW(parsingContext->parseArena, ExpressionGroup);

    group->expression = expression;
    group->type = EXPR_GROUP;
    group->containsOr = parsingContext->orCount > 0;

    parsingContext->andCount = 0;
    parsingContext->orCount = 0;

    return (Expression *)group;
}

Expression *CreateStringExpression(ParsingContext *parsingContext, const char* string)
{
    TermExpression *expression = NEW(parsingContext->parseArena, TermExpression);
    
    expression->value.string = S(string);
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

    if (qualifier != NULL)
    {
        expression->value.identifier.qualifier = S(qualifier);
    }
    
    expression->value.identifier.name = S(name);
    expression->type = EXPR_IDENIFIER;

    *Push(&parsingContext->referenced, parsingContext->parseArena) = &expression->value.identifier;

    return (Expression *)expression;
}

Expression *CreateAndExpression(ParsingContext *parsingContext, Expression *left, Expression *right)
{
    InfixExpression *and = NEW(parsingContext->parseArena, InfixExpression);

    and->left = left;
    and->right = right;
    and->type = EXPR_AND;

    parsingContext->andCount++;

    return (Expression *)and;
}

Expression *CreateOrExpression(ParsingContext *parsingContext, Expression *left, Expression *right)
{
    InfixExpression *or = NEW(parsingContext->parseArena, InfixExpression);

    or->left = left;
    or->right = right;
    or->type = EXPR_OR;

    parsingContext->orCount++;

    return (Expression *)or;
}

Expression *CreateInfixExpression(ParsingContext *parsingContext, ExpressionType expressionType, Expression *left, Expression *right)
{
    InfixExpression *expression = NEW(parsingContext->parseArena, InfixExpression);
    
    expression->left = left;
    expression->right = right;
    expression->type = expressionType;

    if (expressionType == EXPR_ADD)
    {
        parsingContext->andCount++;
    }

    if (expressionType == EXPR_OR)
    {
        parsingContext->orCount++;
    }

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

Selection *CreateSelection(ParsingContext *parsingContext, Expression *where)
{
    Selection *selection = NEW(parsingContext->parseArena, Selection);

    selection->condition = where;
    selection->type = LPLAN_SELECT;


    if (where->type != EXPR_GROUP)
    {
        selection->condition = CreateExpressionGroup(parsingContext, where);
    }
    else
    {
        selection->condition = where;
    }
    parsingContext->selection = selection;

    return selection;
}
