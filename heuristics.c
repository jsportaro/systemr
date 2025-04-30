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

static void AddToFilter(LogicalScan *log, Expression *expression, Arena *arena)
{
}

static void AddScanArgument(LogicalScanLookup **scansLookup, Identifier *identifier, Arena *arena)
{
    LogicalScan *scan = ScanLookup(scansLookup, identifier->attribute->relationId);
        
    if (AddScanArgumentLookup(&scan->scanArgumentsLookup, identifier->attribute->id, arena) == true)
    {
        *Push(&scan->scanArguments, arena) = identifier->attribute;
    }
}

static void PushDownProjections(Plan *plan, Arena *arena)
{
    LogicalProjection *projection = plan->projections->first;

    while (projection != NULL)
    {
        Identifier *identifier = projection->identifiers;

        while (identifier != NULL)
        {
            AddScanArgument(&plan->scansLookup, identifier, arena);
            
            identifier = identifier->next;
        }

        projection = (LogicalProjection *)projection->child;
    }
}

static Expression *RewriteSelection(Expression *expression, Plan *plan, Arena *arena)
{
    switch (expression->type)
    {
        case EXPR_IDENIFIER: {
            TermExpression *identifierExpression = (TermExpression *)expression;

            AddScanArgument(&plan->scansLookup, &(identifierExpression->value.identifier), arena);

            return expression;
        }
        case EXPR_ADD:
        case EXPR_SUB:
        case EXPR_MUL:
        case EXPR_DIV: {
            InfixExpression *infix = (InfixExpression *)expression;

            RewriteSelection(infix->left, plan, arena);
            RewriteSelection(infix->right, plan, arena);

            return expression;
        }
        case EXPR_EQU: {
            InfixExpression *infix = (InfixExpression *)expression;
            
            infix->left = RewriteSelection(infix->left, plan, arena);
            infix->right = RewriteSelection(infix->right, plan, arena);

            if (infix->left->type == EXPR_IDENIFIER && infix->right->type == EXPR_IDENIFIER)
            {
                //  This is a join
                TermExpression *leftExpression = (TermExpression *)infix->left;
                TermExpression *rightExpression = (TermExpression *)infix->right;
            }
            else if (infix->left->type == EXPR_IDENIFIER)
            {
                //  This is a filter
                TermExpression *id = (TermExpression *)expression;
                LogicalScan *scan = ScanLookup(&plan->scansLookup, id->value.identifier.attribute->relationId);

                AddToFilter(scan, expression, arena);
            }

            return infix;
        }
        case EXPR_AND:
            break;
        case EXPR_OR:
            //  ORs kind of suck - I think the only time I can push them down
            //  is when the entire OR expression is limited to one table
            //  Otherwise, you gotta leave them till after all join of all
            //  referenced tables is done.  I think I'm just going to leave them
            //  till all the joins are done.
            break;
        case EXPR_IN_QUERY:
            break;   
    }
}

static void PushDownSelection(Plan *plan, Arena *arena)
{
    RewriteSelection(plan->selection->condition, plan, arena);
}

bool ApplyHeuristics(Plan *plan, Arena *arena)
{
    PushDownProjections(plan, arena);
    PushDownSelection(plan, arena);
    return true;
}