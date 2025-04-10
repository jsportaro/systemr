#include <arena.h>
#include <common.h>
#include <parser.h>
#include <parser.gen.h>
#include <lexer.gen.h>
#include <rstrings.h>

typedef struct Alias Alias;

struct Alias
{
    Alias *child[4];
    String name;
};

static bool AddAlias(Alias **aliases, String alias, Arena *arena)
{
    uint64_t hash = 0;
    for (hash = HashString(alias); *aliases; hash <<= 2)
    {
        if (Equals(alias, (*aliases)->name) == true)
        {
            return false;
        }

        aliases = &(*aliases)->child[hash >> 62];
    }

    if (arena == NULL)
    {
        return false;
    }

    *aliases = NEW(arena, Alias);
    (*aliases)->name = alias;
    return true;
}

static bool AliasExists(Alias **aliases, String alias)
{
    uint64_t hash = 0;
    for (hash = HashString(alias); *aliases; hash <<= 2)
    {
        if (Equals(alias, (*aliases)->name) == true)
        {
            return true;
        }

        aliases = &(*aliases)->child[hash >> 62];
    }

    return false;
}

static bool BuildAliasLookup(LogicalScan *scan, Alias **aliases, Arena *arena)
{
    bool success = true;

    while (scan != NULL)
    {
        success &= AddAlias(aliases, scan->alias, arena);

        //  Todo:  Add some kind of messaging
        scan = scan->next;
    }

    return success;
}

static bool VerifyUnresolvedIdentifiers(Alias **aliases, Identifier *unresolved)
{
    //  Look through Identifiers to see if they match up with a table alias.
    //  For identifiers with a qualifier
    //     -> If matches a alias; replace qualifier with table name
    //     -> If matches neither table or alias; error!
    bool success = true;
    while (unresolved != NULL)
    {
        if (unresolved->qualifier.length != 0)
        {
            success &= AliasExists(aliases, unresolved->qualifier);
        }

        unresolved = unresolved->next;
    }

    return success;
}

static bool VerifyAliasedSelections(Alias **aliases, LogicalSelection *selection)
{
    if (selection != NULL)
    {
        return VerifyUnresolvedIdentifiers(aliases, selection->unresolved);
    }

    return true;
}

static bool VerifyAliasedProjections(ParsingContext *parsingContext, Alias **aliases, LogicalSelection **selection)
{
    PlanNode *node = parsingContext->plan->root;
    bool verified = true;
    while (node->type == LPLAN_PROJECT_ALL || node->type == LPLAN_PROJECT)
    {
        LogicalProjection *projection = (LogicalProjection *)node;

        verified &= VerifyUnresolvedIdentifiers(aliases, projection->unresolved);
        node = projection->child;
    }

    if (node->type == LPLAN_SELECT)
    {
        *selection = (LogicalSelection *)node;
    }

    return verified;
}

static void ParsePostProcessing(ParsingContext *parsingContext)
{
    LogicalSelection *selection = NULL;
    parsingContext->success = true;

    Alias *aliases = NULL;

    if (BuildAliasLookup(parsingContext->scans, &aliases, parsingContext->parseArena) == false)
    {
        parsingContext->success = false;

        return;
    }

    // Check to make sure any identifiers like 'alias.column' actually use a 
    // alias present in the from clause
    parsingContext->success &= VerifyAliasedProjections(parsingContext, &aliases, &selection);
    parsingContext->success &= VerifyAliasedSelections(&aliases, selection);

    memset(parsingContext->aliasLookup, 0, MAX_HASH_SIZE * sizeof(TableReference *)); 

}

static void ParseBaseGrammar(ParsingContext *parsingContext, const char *sql, size_t length)
{
    yyscan_t scanInfo = {0};
    
    if(yylex_init(&scanInfo))
    {
        parsingContext->success = false;
        return;
    }

    yy_scan_bytes(sql, length, scanInfo);
    yyparse(scanInfo, parsingContext);
    yylex_destroy(scanInfo);
}

void ParseSQL(ParsingContext *parsingContext, const char *sql, size_t length)
{
    ParseBaseGrammar(parsingContext, sql, length);
    ParsePostProcessing(parsingContext);

    //  At this point we know if SQL is at least consistent with itself.  
    //  For example consider:
    //     SELECT p.name FROM people p WHERE p.name = 'joe';
    //  will pass all checks
    //  Whereas:
    //     SELECT nope.name FROM people p WHERE p.name = 'joe';
    //  will abort execution (for now)
    //  Also, we won't have duplicate table names or aliases
}
