#include <arena.h>
#include <common.h>
#include <parser.h>
#include <parser.gen.h>
#include <lexer.gen.h>

static bool AddAliasLookup(ParsingContext *parsingContext, LogicalScan *scan, const char *alias)
{
    size_t aliasLength = strlen(alias);
    uint32_t i = Hash(alias, aliasLength) % MAX_HASH_SIZE;

    //  If neither name exists, we're good to add.
    //  However, if a reference already exists in either case then we have a duplicate name collision
    for (;;)
    {
        if (parsingContext->aliasLookup[i] == NULL)
        {
            parsingContext->aliasLookup[i] = scan;

            return true;
        }
        else if (strncmp(alias, parsingContext->aliasLookup[i]->alias, aliasLength) == 0)
        {
            //  Alias already exists
            return false;
        }

        i = (i + 1) % MAX_HASH_SIZE;
    }
}

static void BuildAliasLookup(ParsingContext *parsingContext)
{
    LogicalScan *scan = parsingContext->scans;
    bool wasAdded = false;

    while (scan != NULL)
    {
        parsingContext->success &= wasAdded = AddAliasLookup(parsingContext, scan, scan->alias);

        //  Todo:  Add some kind of messaging
        scan = scan->next;
    }
}

static bool VerifyUnresolvedIdentifiers(ParsingContext *parsingContext, Identifier *unresolved)
{
    //  Look through Identifiers to see if they match up with a table alias.
    //  For identifiers with a qualifier
    //     -> If matches a alias; replace qualifier with table name
    //     -> If matches neither table or alias; error!
    bool success = true;
    while (unresolved != NULL)
    {
        if (unresolved->qualifier != NULL)
        {
            size_t length = strlen(unresolved->qualifier);
            uint32_t i = Hash(unresolved->qualifier, length) % MAX_HASH_SIZE;

            for (;;)
            {
                if (parsingContext->aliasLookup[i] == NULL)
                {
                    success &= false;
                    break;
                }
                else if (strncmp(unresolved->qualifier, parsingContext->aliasLookup[i]->alias, length) == 0)
                {
                    success &= true;
                    break;
                }

                i = (i + 1) % MAX_HASH_SIZE;
            }
        }

        unresolved = unresolved->next;
    }

    return success;
}

static bool VerifyAliasedSelections(ParsingContext *parsingContext, LogicalSelection *selection)
{
    if (selection != NULL)
    {
        return VerifyUnresolvedIdentifiers(parsingContext, selection->unresolved);
    }

    return true;
}

static bool VerifyAliasedProjections(ParsingContext *parsingContext, LogicalSelection **selection)
{
    PlanNode *node = parsingContext->plan->root;
    bool verified = true;
    while (node->type == LPLAN_PROJECT_ALL || node->type == LPLAN_PROJECT)
    {
        LogicalProjection *projection = (LogicalProjection *)node;

        verified &= VerifyUnresolvedIdentifiers(parsingContext, projection->unresolved);
        node = projection->child;
    }

    if (node->type == LPLAN_SELECT)
    {
        *selection = (LogicalSelection *)node;
    }

    return verified;
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

static void ParsePostProcessing(ParsingContext *parsingContext)
{
    LogicalSelection *selection = NULL;
    parsingContext->success = true;

    BuildAliasLookup(parsingContext);

    // Check to make sure any identifiers like 'alias.column' actually use a 
    // alias present in the from clause
    parsingContext->success &= VerifyAliasedProjections(parsingContext, &selection);
    parsingContext->success &= VerifyAliasedSelections(parsingContext, selection);

    memset(parsingContext->aliasLookup, 0, MAX_HASH_SIZE * sizeof(TableReference *)); 

}

void ParseSQL(ParsingContext *parsingContext, const char *sql, size_t length)
{

    ParseBaseGrammar(parsingContext, sql, length);
    ParsePostProcessing(parsingContext);

    //  At this point the SQL is at least consistent with itself.  
    //  For example consider:
    //     SELECT p.name FROM people p WHERE p.name = 'joe';
    //  will pass all checks
    //  Whereas:
    //     SELECT nope.name FROM people p WHERE p.name = 'joe';
    //  will abort execution (for now)
}
