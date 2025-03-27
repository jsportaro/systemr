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


ParsingContext ParseSQL(const char *sql, size_t length, Arena arena)
{
    ParsingContext parsingContext = {0};
    parsingContext.parseArena = arena;
    parsingContext.success = true;

    ParseBaseGrammar(&parsingContext, sql, length);

    return parsingContext;
}
