#include <heuristics.h>
#include <slice.h>

static bool AddScanArgumentLookup(ScanArgumentLookup **scanArumentLookup, int attributeId, Arena *arena)
{
    for (uint64_t h = attributeId; *scanArumentLookup; h <<= 2)
    {
        if (attributeId == (*scanArumentLookup)->attributeId)
        {
            return false;
        }
        scanArumentLookup = &(*scanArumentLookup)->child[h >> 62];
    }

    if (!arena) 
    {
        return false;
    }

    *scanArumentLookup = NEW(arena, ScanArgumentLookup);
    (*scanArumentLookup)->attributeId = attributeId;
    return true; 
}  

static void AddToFilter(Scan *scan, InfixExpression *expression, Arena *arena)
{
    if (scan->filter == NULL)
    {
        scan->filter = expression;
    }
    else
    {
        InfixExpression *and = NEW(arena, InfixExpression);
        and->left = (Expression *)expression;
        and->right = (Expression *)scan->filter;
        and->type = EXPR_AND;
        scan->filter = and;
    }
}

static void AddScanArgument(ScanLookup **scansLookup, Identifier *identifier, Arena *arena)
{
    Scan *scan = GetScan(scansLookup, identifier->attribute->relation->id);
        
    if (AddScanArgumentLookup(&scan->argumentsLookup, identifier->attribute->id, arena) == true)
    {
        *Push(&scan->arguments, arena) = identifier->attribute;
    }
}

static Expression *RewriteSelection(Expression *expression, Plan *plan, Arena *arena)
{
    switch (expression->type)
    {
        case EXPR_IDENIFIER: {
            //TermExpression *identifierExpression = (TermExpression *)expression;

            //AddScanArgument(&plan->scansLookup, &(identifierExpression->value.identifier), arena);

            return expression;
        }
        case EXPR_NUMBER: {
            return expression;
        }
        case EXPR_STRING: {
            return expression;
        }
        case EXPR_ADD:
        case EXPR_SUB:
        case EXPR_MUL:
        case EXPR_DIV:
            return expression;
        case EXPR_EQU: {
            InfixExpression *infix = (InfixExpression *)expression;
            
            infix->left = RewriteSelection(infix->left, plan, arena);
            infix->right = RewriteSelection(infix->right, plan, arena);

            if (infix->left->type == EXPR_IDENIFIER && infix->right->type == EXPR_IDENIFIER)
            {
                //  This is a join
                TermExpression *leftExpression = (TermExpression *)infix->left;
                TermExpression *rightExpression = (TermExpression *)infix->right;

                UNUSED(leftExpression);
                UNUSED(rightExpression);
                
                return NULL;
            }
            else if (infix->left->type == EXPR_IDENIFIER)
            {
                //  This is a filter
                TermExpression *id = (TermExpression *)infix->left;
                Scan *scan = GetScan(&plan->scansLookup, id->value.identifier.attribute->relation->id);

                AddToFilter(scan, infix, arena);
 
                return NULL;
            }

            return (Expression *)infix;
        }
        case EXPR_AND: {
            InfixExpression *infix = (InfixExpression *)expression;

            Expression *left = RewriteSelection(infix->left, plan, arena);
            Expression *right = RewriteSelection(infix->right, plan, arena);

            if (left == NULL && right == NULL)
            {
                return NULL;
            }
            else if (left == NULL)
            {
                return right;
            }
            else if (right == NULL)
            {
                return left;
            }
            else
            {
                return expression;
            }
        }
        case EXPR_OR:
            //  ORs kind of suck - I think the only time I can push them down
            //  is when the entire OR expression is limited to one table
            //  Otherwise, you gotta leave them till after all join of all
            //  referenced tables is done.  I think I'm just going to leave them
            //  till all the joins are done.
            return expression;
        case EXPR_IN_QUERY:
            //  Not supported yet
            abort();
            break;
        case EXPR_GROUP: {
            ExpressionGroup *group = (ExpressionGroup *)expression;

            if (group->containsOr == true)
            {
                return expression;
            }

            //  There are ways to push down further but I'm stopping here

            return RewriteSelection(group->expression, plan, arena);
        }
    }

    //  Bug territory
    abort();
}

static void PushDownSelection(Plan *plan, Arena *arena)
{
    plan->selection->condition = RewriteSelection(plan->selection->condition, plan, arena);
}

static void PushDownScanArguments(Plan *plan, Arena *arena)
{
    if (plan->pushDown == false)
    {
        return;
    }

    for (int i = 0; i < plan->referenced.length; i++)
    {
        Identifier *identifier = plan->referenced.data[i];

        AddScanArgument(&plan->scansLookup, identifier, arena);
    }
}

bool ApplyHeuristics(Plan *plan, Arena *arena)
{
    PushDownScanArguments(plan, arena);
    PushDownSelection(plan, arena);

    return true;
}