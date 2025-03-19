#include <parser.h>

#include <stdio.h>
#include <string.h>

int main(void)
{
    printf("SystemR\n");
    char* sql = "select a as column1, b as column2 from table1, table2;";

    ParseSQL(sql, strlen(sql));    

    return 0;
}
