#include <parser.h>

#include <stdio.h>
#include <string.h>

int main(void)
{
    printf("SystemR\n");
    char* sql = "select * from table;";

    parse_sql(sql, strlen(sql));    

    return 0;
}