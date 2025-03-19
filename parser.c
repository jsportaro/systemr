#include <arena.h>
#include <parser.h>
#include <parser.gen.h>
#include <lexer.gen.h>

int ParseSQL(char *sql, size_t length)
{
    yyscan_t scanInfo = {0};
    ParsingContext context = {0};
    context.parseArena = NewArena(MAX_EXPRESSIONS);
    
    if(yylex_init(&scanInfo))
    {
        return 1;
    }

    yy_scan_bytes(sql, length, scanInfo);
    yyparse(scanInfo, &context);
    yylex_destroy(scanInfo);

    return 0;
}
