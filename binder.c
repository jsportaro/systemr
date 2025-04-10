#include <binder.h>
#include <catalog.h>
#include <parser.h>
#include <plan.h>

#include <string.h>

typedef struct AliasBinding AliasBinding;

struct AliasBinding
{
    AliasBinding *child[4];
    String alias;
    Relation *relation;

    AliasBinding *next;
};

static AliasBinding *LookupAlias(AliasBinding **aliasBinding, String alias, Arena *arena)
{
    for (uint64_t h = HashString(alias); *aliasBinding; h <<= 2) {
        if (Equals(alias, (*aliasBinding)->alias)) {
            return *aliasBinding;
        }
        aliasBinding = &(*aliasBinding)->child[h >> 62];
    }

    if (!arena) 
    {
        return NULL;
    }

    *aliasBinding = NEW(arena, AliasBinding);
    (*aliasBinding)->alias = alias;
    return *aliasBinding;
}

static AliasBinding *BindScans(LogicalScan **scans, AliasBinding **aliasLookup, Arena *executionArena)
{
    AliasBinding *last = NULL;

    while (*scans != NULL)
    {
        Relation *relation = GetRelation((*scans)->name);
        
        if (relation != NULL)
        {
            (*scans)->relation = relation;

            AliasBinding *aliasBinding = LookupAlias(aliasLookup, (*scans)->alias, executionArena);
            aliasBinding->relation = relation;
            aliasBinding->next = last;
            last = aliasBinding;

            *scans = (*scans)->next;
            continue;
        }
        scans = &(*scans)->next;
    }

    return last;
}

static void AttemptBindProjections(Plan *plan)
{
    PlanNode *current = plan->root;

    while (current != NULL)
    {
        if (current->type != LPLAN_PROJECT || current->type != LPLAN_PROJECT_ALL)
        {
            break;
        }
    }
}

bool AttemptBind(Plan *plan, Arena *executionArena)
{
    AliasBinding *aliasLookup = { 0 };
    AliasBinding *boundAliases = BindScans(&plan->scans, &aliasLookup, executionArena);

    UNUSED(boundAliases);

    if (plan->scans != NULL)
    {
        //  Couldn't bind everything
        return false;
    }

    AttemptBindProjections(plan);

    return true;
}
