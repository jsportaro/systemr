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
        //sql = "SELECT person.name FROM person, place Where name = 'joe' and person.address_id = place.id or name = 'mike'";
        //sql = "SELECT person.name FROM person, place Where name = 'joe' and (person.address_id = 1 or name = 'mike')";
        sql = "SELECT person.name FROM person, place Where person.address_id = place.id and (name = 'joe' or name = 'mike')";
        //sql = "SELECT person.name FROM person, place Where name = 'joe' and person.address_id = place.id;";
        //sql = "SELECT person.name FROM person, place Where age = 1 and (person.name = 'joe' or person.name='mike') and person.address_id = place.id and age = 2;";
    }
    else
    {
        sql = argv[1];
    }

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

    printf("Heuristics\n");

    String s = S("");
    s = StringifyExpression(s, parsingContext.plan->selection->condition, &executionArena);
    printf("Before Heuristics WHERE clause is - '%.*s'\n", (int)s.length, s.data);

    ApplyHeuristics(parsingContext.plan, &executionArena);

    s = S("");
    s = StringifyExpression(s, parsingContext.plan->selection->condition, &executionArena);
    printf("After Heuristics WHERE clause is - '%.*s'\n", (int)s.length, s.data);

    LogicalScan *scan = parsingContext.plan->scans;

    while (scan != NULL)
    {
        if (scan->filter != NULL)
        {
            String filter = S("");
            filter = StringifyExpression(filter, scan->filter, &executionArena);
            printf(
                "Filter for '%.*s' - '%.*s'\n",
                (int)scan->name.length,
                scan->name.data,
                (int)filter.length,
                filter.data);
        }
        scan = scan->next;
    }
    
Cleanup:
    free(executionArena.original);
    executionArena.begin = NULL;
    executionArena.end = NULL;

    return 0;
}
