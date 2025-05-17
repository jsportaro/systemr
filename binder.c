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
    for (uint64_t h = HashString(alias); *aliasBinding; h <<= 2)
    {
        if (Equals(alias, (*aliasBinding)->alias))
        {
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

static ScanLookup *AddScanLookup(ScanLookup **scansLookup, Scan *scan, Arena *arena)
{
    uint64_t h = scan->relation->id;
    while (*scansLookup)
    {
        if (scan->relation->id == (*scansLookup)->relationId)
        {
            return *scansLookup;
        }
        scansLookup = &(*scansLookup)->child[h >> 62];

        h <<= 2;
    }

    if (!arena) 
    {
        return NULL;
    }

    *scansLookup = NEW(arena, ScanLookup);
    (*scansLookup)->relationId = scan->relation->id;
    (*scansLookup)->scan = scan;
    
    return *scansLookup; 
}

static bool BindScans(ScanLookup **scansLookup, ScanList scanList, AliasBinding **aliasLookup, Arena *executionArena)
{
    bool success = true;
    
    for (int i = 0; i < scanList.length; i ++)
    {
        Scan *scan = scanList.data[i];

        Relation *relation = GetRelation(scan->name);
        
        if (relation != NULL)
        {
            scan->relation = relation;
            AliasBinding *aliasBinding = LookupAlias(aliasLookup, scan->alias, executionArena);
            AddScanLookup(scansLookup, scan, executionArena);
            aliasBinding->relation = relation;
        }
        else
        {
            //  Error condition - can't find a relation
            fprintf(stderr, "Error - could not find relation %.*s\n", (int)scan->name.length, scan->name.data);
            success &= false;
        }
    }

    return success;
}

static bool AttemptBindWithAlias(Identifier **unresolved, AliasBinding **aliasLookup)
{
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

static bool AttemptBindAnyRelation(Identifier **unresolved, ScanList scans)
{
    bool ambiguous = false;
    bool found = false;

    while (*unresolved != NULL)
    {
        ambiguous = false;
        found = false;

        for (int i = 0; i < scans.length; i++)
        {
            Scan *current = scans.data[i];

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

        if (found == true && ambiguous == false)
        {
            *unresolved = (*unresolved)->next;
        }
        else
        {
            unresolved = &(*unresolved)->next;
        }
    }

    return *unresolved == NULL;
}

static bool AttemptBindProjection(Projection *projection, AliasBinding **aliasLookup, ScanList scans)
{
    return AttemptBindWithAlias(&projection->unresolved, aliasLookup) &&
           AttemptBindAnyRelation(&projection->unresolved, scans);
}

static bool AttemptBindProjections(Plan *plan, AliasBinding **aliasLookup, ScanList scans)
{
    Projection *current = plan->projections->first;
    bool success = true;

    while (current != NULL)
    {
        if (current->type == LPLAN_PROJECT)
        {
            success &= AttemptBindProjection((Projection *)current, aliasLookup, scans);
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
            current = (Projection *)(current)->child;
    }

    return success;
}

static bool AttemptBindSelection(Selection *selection, AliasBinding **aliasLookup, ScanList scans)
{
    if (selection == NULL)
    {
        return true;
    }

    return AttemptBindWithAlias(&selection->unresolved, aliasLookup) &&
           AttemptBindAnyRelation(&selection->unresolved, scans);
}

bool AttemptBind(Plan *plan, Arena *executionArena)
{
    AliasBinding *aliasLookup = { 0 };
    bool success = true;

    // Now, we'll try to bind all the identifiers to attributes or relations.
    // If something fails, continue on to give the user as much info as we can before
    // bombing out 
    success &= BindScans(&plan->scansLookup, plan->scanList, &aliasLookup, executionArena);
    success &= AttemptBindProjections(plan, &aliasLookup, plan->scanList);
    success &= AttemptBindSelection(plan->selection, &aliasLookup, plan->scanList);

    return success;
}
