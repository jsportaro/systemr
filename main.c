#include <parser.h>

#include <stdio.h>
#include <string.h>

int main(void)
{
    printf("SystemR\n");
    char* sql = "select * from table1, table2;";

    parse_sql(sql, strlen(sql));    

    return 0;
}