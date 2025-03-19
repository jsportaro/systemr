#include <arena.h>
#include <common.h>
#include <parser.h>
#include <parser.gen.h>
#include <lexer.gen.h>

ParsingContext ParseSQL(char *sql, size_t length, Arena arena)
{
    yyscan_t scanInfo = {0};
    ParsingContext context = {0};
    context.parseArena = arena;
    
    if(yylex_init(&scanInfo))
    {
        abort();
    }

    yy_scan_bytes(sql, length, scanInfo);
    yyparse(scanInfo, &context);
    yylex_destroy(scanInfo);

    return context;
}
