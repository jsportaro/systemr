#include <catalog.h>
#include <common.h>
#include <binder.h>
#include <heuristics.h>
#include <parser.h>
#include <plan.h>

#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
    printf("SystemR\n");
    char *sql;
    if (argc != 2)
    {
        //sql = "SELECT person.name FROM person, place Where name = 'joe' and person.address_id = place.id;";
        sql = "SELECT person.name FROM person, place Where person.address_id = place.id;";
    }
    else
    {
        sql = argv[1];
    }

    String s = S("");

    printf("Optimizing %s\n", sql);
    Arena executionArena = NewArena(EXECUTION_ARENA_SIZE);
    ParsingContext parsingContext = { 0 };
    bool success = true;
    BuildCatalog(&executionArena);
    parsingContext.parseArena = &executionArena;

    ParseSQL(&parsingContext, sql, strlen(sql));
    printf("Parsing");
    printf(" -- %s\n", parsingContext.success == true ? "success" : "failure");

    if (parsingContext.success == false)
    {
        goto Cleanup;
    }

    printf("Binding");
    success &= success == true ? AttemptBind(parsingContext.plan, &executionArena) : success;
    printf(" -- %s\n", success == true ? "success" : "failure");

    printf("Heuristics");
    success &= success == true ? ApplyHeuristics(parsingContext.plan, &executionArena) : success;
    printf(" -- %s\n", success == true ? "success" : "failure");

    s = StringifyExpression(s, parsingContext.plan->selection->condition, &executionArena);
    printf("After Heuristics WHERE clause is - '%.*s'\n", (int)s.length, s.data);

    
Cleanup:
    free(executionArena.original);
    executionArena.begin = NULL;
    executionArena.end = NULL;

    return 0;
}
