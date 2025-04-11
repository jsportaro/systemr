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

static bool BindScans(LogicalScan *scans, AliasBinding **aliasLookup, Arena *executionArena)
{
    bool success = true;
    
    while (scans != NULL)
    {
        Relation *relation = GetRelation(scans->name);
        
        if (relation != NULL)
        {
            scans->relation = relation;

            AliasBinding *aliasBinding = LookupAlias(aliasLookup, scans->alias, executionArena);
            aliasBinding->relation = relation;
        }
        else
        {
            //  Error condition - can't find a relation
            fprintf(stderr, "Error - could not find relation %.*s\n", (int)scans->name.length, scans->name.data);
            success &= false;
        }

        scans = scans->next;
    }

    return success;
}

static bool AttemptBindWithAlias(LogicalProjection *projection, AliasBinding **aliasLookup)
{
    Identifier **unresolved = &projection->unresolved;
    bool success = true;

    while (*unresolved != NULL)
    {
        if ((*unresolved)->qualifier.length > 0)
        {
            AliasBinding *aliasBinding = LookupAlias(aliasLookup, (*unresolved)->qualifier, NULL);
            
            if (aliasBinding == NULL)
            {
                //  This is an error condition that should never happen
                //  We checked that
                //  1) The alias exists in the SQL statement
                //  2) All tables have already been found
                success &= false;
                fprintf(stderr, "Error - could not find relation %.*s\n", (int)(*unresolved)->qualifier.length, (*unresolved)->qualifier.data);
                goto Next;
            }

            Attribute *attribute = GetAttribute(aliasBinding->relation, (*unresolved)->name);

            if (attribute == NULL)
            {
                //  This is a _real_ error condition
                success &= false;
                fprintf(stderr, "Error - could not find attribute %.*s\n", (int)(*unresolved)->name.length, (*unresolved)->name.data);
                goto Next;   
            }

            (*unresolved)->attribute = attribute;

            *unresolved = (*unresolved)->next;

            continue;
        }
        
        Next:
            unresolved = &(*unresolved)->next;
    }

    return success;
}

static bool AttemptBindAnyRelation(LogicalProjection *projection, LogicalScan *scans)
{
    Identifier **unresolved = &projection->unresolved;
    bool ambiguous = false;
    bool found = false;

    while (*unresolved != NULL)
    {
        LogicalScan *current = scans;

        ambiguous = false;
        found = false;

        while (current != NULL)
        {
            if (current->relation != NULL)
            {
                Attribute *attribute = GetAttribute(current->relation, (*unresolved)->name);

                if (attribute != NULL && (*unresolved)->attribute == NULL)
                {
                    found = true;
                    (*unresolved)->attribute = attribute;
                }
                else if (attribute != NULL && (*unresolved)->attribute != NULL)
                {
                    //  Error conditions - ambiguous identifier (found in more than one relation)
                    fprintf(stderr, "Error - ambiguous attribute %.*s\n", (int)(*unresolved)->name.length, (*unresolved)->name.data);

                    ambiguous = true;
                }
            }

            current = current->next;
        }

        if (found == true && ambiguous == false)
        {
            *unresolved = (*unresolved)->next;
        }
        else
        {
            unresolved = &(*unresolved)->next;
        }
    }

    return projection->unresolved == NULL;
}

static bool AttemptBindProjection(LogicalProjection *projection, AliasBinding **aliasLookup, LogicalScan *scans)
{
    return AttemptBindWithAlias(projection, aliasLookup) &&
           AttemptBindAnyRelation(projection, scans);
}

static bool AttemptBindProjections(Plan *plan, AliasBinding **aliasLookup, LogicalScan *scans, Arena *executionArena)
{
    PlanNode *current = plan->root;
    bool success = true;

    while (current != NULL)
    {
        if (current->type == LPLAN_PROJECT)
        {
            success &= AttemptBindProjection((LogicalProjection *)current, aliasLookup, scans);
            goto Next;
        }
        else if (current->type == LPLAN_PROJECT_ALL)
        {
            goto Next;
        }
        else
        {
            break;
        }

        Next:
            current = ((LogicalProjection *)current)->child;
    }

    return success;
}

bool AttemptBind(Plan *plan, Arena *executionArena)
{
    AliasBinding *aliasLookup = { 0 };
    bool success = true;

    success &= BindScans(plan->scans, &aliasLookup, executionArena);
    success &= AttemptBindProjections(plan, &aliasLookup, plan->scans, executionArena);

    return success;
}
