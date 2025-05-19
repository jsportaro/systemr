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

static bool TryBindWithAlias(Identifier *unresolved, AliasBinding **aliasLookup)
{
    bool success = false;

    if (unresolved->qualifier.length > 0)
    {
        AliasBinding *aliasBinding = LookupAlias(aliasLookup, unresolved->qualifier, NULL);
        
        if (aliasBinding == NULL)
        {
            //  This is an error condition that should never happen
            //  We checked that
            //  1) The alias exists in the SQL statement
            //  2) All tables have already been found
            success &= false;
            fprintf(stderr, "Error - could not find relation %.*s\n", (int)unresolved->qualifier.length, unresolved->qualifier.data);
            goto EndAttempt;
        }

        Attribute *attribute = GetAttribute(aliasBinding->relation, unresolved->name);

        if (attribute == NULL)
        {
            //  This is a _real_ error condition
            success &= false;
            fprintf(stderr, "Error - could not find attribute %.*s\n", (int)unresolved->name.length, unresolved->name.data);
            goto EndAttempt;   
        }

        unresolved->attribute = attribute;
        success = true;
    }
EndAttempt:
    return success;
}

static bool TryBindAnyRelation(Identifier *unresolved, ScanList scans)
{
    bool ambiguous = false;
    bool found = false;

    for (int i = 0; i < scans.length; i++)
    {
        Scan *current = scans.data[i];

        Attribute *attribute = GetAttribute(current->relation, unresolved->name);

        if (attribute != NULL && unresolved->attribute == NULL)
        {
            found = true;
            unresolved->attribute = attribute;
        }
        else if (attribute != NULL && unresolved->attribute != NULL)
        {
            //  Error conditions - ambiguous identifier (found in more than one relation)
            fprintf(stderr, "Error - ambiguous attribute %.*s\n", (int)unresolved->name.length, unresolved->name.data);

            ambiguous = true;
        }
    }

    return found == true && ambiguous == false;
}

static bool BindIdentifiers(Referenced referenced, AliasBinding **aliasLookup, ScanList scans)
{
    bool success = true;

    for (int i = 0; i < referenced.length; i++)
    {
        Identifier *unresolved = referenced.data[i];
        bool couldBind = false;

        couldBind = TryBindWithAlias(unresolved, aliasLookup);

        if (couldBind == false)
        {
            couldBind = TryBindAnyRelation(unresolved, scans);
        }

        success &= couldBind;
    }

    return success;
}

bool AttemptBind(Plan *plan, Arena *executionArena)
{
    AliasBinding *aliasLookup = { 0 };
    bool success = true;

    // Now, we'll try to bind all the identifiers to attributes or relations.
    // If something fails, continue on to give the user as much info as we can before
    // bombing out 
    success &= BindScans(&plan->scansLookup, plan->scanList, &aliasLookup, executionArena);
    success &= BindIdentifiers(plan->referenced, &aliasLookup, plan->scanList);

    return success;
}
