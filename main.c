#include <catalog.h>
#include <common.h>
#include <binder.h>
#include <parser.h>
#include <plan.h>

#include <stdio.h>
#include <string.h>

int main(void)
{
    printf("SystemR\n");
    //char *sql = "SELECT person.name AS FullName, place.city AS Town, zip as Zip FROM person, place, thing WHERE person.address_id = place.id;";
    //char *sql = "SELECT person.name, name FROM person, nope;";
    char *sql = "SELECT p.name, age FROM person p, place WHERE p.name = 'joe';";
    
    Arena executionArena = NewArena(EXECUTION_ARENA_SIZE);
    ParsingContext parsingContext = { 0 };

    BuildCatalog(&executionArena);
    parsingContext.parseArena = &executionArena;

    ParseSQL(&parsingContext, sql, strlen(sql));

    printf("Parsing %s\n", parsingContext.success == true ? "success" : "failure");

    if (parsingContext.success == true)
    {
        AttemptBind(parsingContext.plan, &executionArena);
    }

    free(executionArena.original);
    executionArena.begin = NULL;
    executionArena.end = NULL;

    return 0;
}
