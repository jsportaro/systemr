#include <arena.h>
#include <common.h>
#include <parser.h>
#include <parser.gen.h>
#include <lexer.gen.h>

static bool AddAliasLookup(ParsingContext *parsingContext, TableReference *tableReference, const char *alias)
{
    size_t aliasLength = strlen(alias);
    uint32_t i = Hash(alias, aliasLength) % MAX_HASH_SIZE;

    //  If neither name exists, we're good to add.
    //  However, if a reference already exists in either case then we have a duplicate name collision
    for (;;)
    {
        if (parsingContext->aliasLookup[i] == NULL)
        {
            parsingContext->aliasLookup[i] = tableReference;

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

static bool VerifyAliasedSelections(ParsingContext *parsingContext, WhereExpression *where)
{
    return VerifyUnresolvedIdentifiers(parsingContext, where->unresolved);
}

static bool VerifyAliasedProjections(ParsingContext *parsingContext, SelectExpressionList *selectExpressionList)
{
    for (int i = 0; i < selectExpressionList->selectListCount; i++)
    {
        SelectExpression *expression = selectExpressionList->selectList[i];
        if (expression->unresolved != NULL)
        {
            if (VerifyUnresolvedIdentifiers(parsingContext, expression->unresolved) == false)
            {
                //  Error state
                //  Todo: Figure out how to communicate errors
                abort();
            }
        }
    }

    return true;
}

static void ParseBaseGrammar(ParsingContext *parsingContext, const char *sql, size_t length)
{
    yyscan_t scanInfo = {0};
    
    if(yylex_init(&scanInfo))
    {
        abort();
    }

    yy_scan_bytes(sql, length, scanInfo);
    yyparse(scanInfo, parsingContext);
    yylex_destroy(scanInfo);
}

static void ParsePostProcessing(ParsingContext *parsingContext)
{
    SelectStatement *select = parsingContext->selectStatement;
    bool wasAdded = false;

    for (int i = 0; i < select->tableReferenceList->count; i++)
    {
        TableReference *tableReference = select->tableReferenceList->tableReferences[i];
        parsingContext->success &= wasAdded = AddAliasLookup(parsingContext, tableReference, tableReference->alias);

        if (wasAdded == false)
        {
            //  Attempt to add the same alias
            abort();
        }
    }

    // Check to make sure any identifiers like 'alias.column' actually use a 
    // alias present in the from clause
    parsingContext->success &= VerifyAliasedProjections(parsingContext, select->selectExpressionList);
    parsingContext->success &= VerifyAliasedSelections(parsingContext, select->whereExpression);

    memset(parsingContext->aliasLookup, 0, MAX_HASH_SIZE * sizeof(TableReference *)); 

}

ParsingContext ParseSQL(const char *sql, size_t length, Arena arena)
{
    ParsingContext parsingContext = {0};
    parsingContext.parseArena = arena;
    parsingContext.success = true;

    ParseBaseGrammar(&parsingContext, sql, length);
    ParsePostProcessing(&parsingContext);

    //  At this point the SQL is at least consistent with itself.  
    //  For example consider:
    //     SELECT p.name FROM people p WHERE p.name = 'joe';
    //  At this point of execution the SQL is verified that all alias
    //  match the FROM clause.  Whereas:
    //     SELECT nope.name FROM people p WHERE p.name = 'joe';
    //  will abort execution (for now)
    return parsingContext;
}
