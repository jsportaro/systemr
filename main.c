#include <parser.h>

#include <string.h>

int main(void)
{
    char* sql = "select * from table;";

    parse_sql(sql, strlen(sql));    

    return 0;
}