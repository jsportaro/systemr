#include <lexer.h>
#include <parser.gen.h>
#include <parser.h>

void parse_sql(char *sql, size_t length)
{
    yyscan_t sc;

    yylex_init(&sc);
    yy_scan_bytes(sql, length, sc);
    yylex_destroy(sc);
}