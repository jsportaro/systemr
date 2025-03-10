#include <parser.gen.h>
#include <lexer.gen.h>
#include <parser.h>

int parse_sql(char *sql, size_t length)
{
    yyscan_t scanInfo = {0};
    ParsingContext context = {0};
    if(yylex_init(&scanInfo))
    {
        return 1;
    }

    yy_scan_bytes(sql, length, scanInfo);
    yyparse(&scanInfo, &context);
    yylex_destroy(scanInfo);

    return 0;
}