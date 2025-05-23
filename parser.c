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

static bool BuildAliasLookup(ScanList scans, Alias **aliases, Arena *arena)
{
    bool success = true;

    for (int i = 0; i < scans.length; i++)
    {
        success &= AddAlias(aliases, scans.data[i]->alias, arena);
    }

    return success;
}

static bool VerifyAliasedIdentifiers(Plan *plan, Alias **aliases)
{
    bool verified = true;

    // Check to make sure any identifiers like 'alias.column' actually use a 
    // alias present in the from clause
    for (int i = 0; i < plan->referenced.length; i++)
    {
        Identifier *unresolved = plan->referenced.data[i];

        if (unresolved->qualifier.length > 0)
        {
            verified &= AliasExists(aliases, unresolved->qualifier);
        }
    }

    return verified;
}

static void ParsePostProcessing(Plan *plan, Arena *parseArena, bool *success)
{
    Alias *aliases = NULL;

    // Create a lookup table for aliases
    // Remember back during BISONing, when an alias didn't exist for a table we assigned the alias
    // as the table name anyway.
    // That's for cases like:
    //      SELECT p.name, address.line_one FROM people p, address
    // We only need one look up table for resolution of address.line_one
    if (BuildAliasLookup(plan->scanList, &aliases, parseArena) == false)
    {
        *success &= false;

        return;
    }

    *success &= VerifyAliasedIdentifiers(plan, &aliases);

    return;
}

static void ParseBaseGrammar(ParsingContext *parsingContext, const char *sql, size_t length)
{
    yyscan_t scanInfo = {0};
    
    if(yylex_init(&scanInfo))
    {
        parsingContext->success &= false;
        return;
    }

    yy_scan_bytes(sql, length, scanInfo);
    yyparse(scanInfo, parsingContext);
    yylex_destroy(scanInfo);
}

void ParseSQL(ParsingContext *parsingContext, const char *sql, size_t length)
{
    parsingContext->success = true;
    ParseBaseGrammar(parsingContext, sql, length);
    ParsePostProcessing(parsingContext->plan, parsingContext->parseArena, &parsingContext->success);

    //  At this point we know if SQL is at least consistent with itself.  
    //  For example consider:
    //     SELECT p.name FROM people p WHERE p.name = 'joe';
    //  will pass all checks
    //  Whereas:
    //     SELECT nope.name FROM people p WHERE p.name = 'joe';
    //  will abort execution (for now)
    //  Also, we won't have duplicate table names or aliases
}
