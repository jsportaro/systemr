#include <arena.h>
#include <common.h>
#include <parser.h>
#include <parser.gen.h>
#include <lexer.gen.h>

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

static void PostParsing(ParsingContext *parsingContext)
{
    //  Look through Identifiers to see if they match up
    //  with table names.
    //  For identifiers with a qualifier
    //     -> If matches a table; skip
    //     -> If matches a alias; replace qualifier with table name
    //     -> If matches neither table or alias; error!
    SelectStatement *select =  parsingContext->selectStatement;

    for (int i = 0; i < select->selectExpressionList->selectListCount; i++)
    {
        SelectExpression *expression = select->selectExpressionList->selectList[i];
        Identifier *unresolved = expression->unresolved;
        while (unresolved != NULL)
        {
            if (unresolved->qualifier != NULL)
            {
                int i = 1;
            }

            unresolved = unresolved->next;
        }
    }
}

ParsingContext ParseSQL(const char *sql, size_t length, Arena arena)
{
    ParsingContext parsingContext = {0};
    parsingContext.parseArena = arena;

    ParseBaseGrammar(&parsingContext, sql, length);
    PostParsing(&parsingContext);

    return parsingContext;
}
