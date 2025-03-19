#include <catalog.h>
#include <common.h>
#include <parser.h>

#include <stdio.h>
#include <string.h>

int main(void)
{
    printf("SystemR\n");
    char* sql = "SELECT name AS FullName, city AS Town FROM person, place WHERE person.;";
    
    BuildCatalog();
    Arena executionArena = NewArena(EXECUTION_ARENA_SIZE);
    ParsingContext parserContext = ParseSQL(sql, strlen(sql), executionArena);    

    free(executionArena.begin);
    executionArena.begin = NULL;
    executionArena.end = NULL;

    UNUSED(parserContext);

    return 0;
}
