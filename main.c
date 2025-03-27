#include <catalog.h>
#include <common.h>
#include <binder.h>
#include <parser.h>

#include <stdio.h>
#include <string.h>

int main(void)
{
    printf("SystemR\n");
    //char *sql = "SELECT person.name AS FullName, place.city AS Town, zip as Zip FROM person, place WHERE person.address_id = place.id;";
    //char *sql = "SELECT person.name, name, a.name FROM person;";
    char *sql = "SELECT name, line_one as addr FROM person as abc WHERE address_id IN (SELECT id FROM address);";

    BuildCatalog();
    Arena executionArena = NewArena(EXECUTION_ARENA_SIZE);
    ParsingContext parserContext = ParseSQL(sql, strlen(sql), executionArena);    
    Plan *plan = AttemptBind(parserContext.selectStatement, NULL, executionArena);

    free(executionArena.begin);
    executionArena.begin = NULL;
    executionArena.end = NULL;

    UNUSED(parserContext);

    return 0;
}
