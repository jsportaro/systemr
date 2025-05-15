#include <catalog.h>
#include <common.h>
#include <binder.h>
#include <heuristics.h>
#include <parser.h>
#include <plan.h>

#include <stdio.h>
#include <string.h>

void Optimize(char *sql, Arena optimizeArena)
{
    printf("Optimizing %s\n", sql);
    ParsingContext parsingContext = { 0 };
    bool success = true;

    parsingContext.parseArena = &optimizeArena;

    ParseSQL(&parsingContext, sql, strlen(sql));
    printf("Parsing");
    printf(" -- %s\n", parsingContext.success == true ? "success" : "failure");

    if (parsingContext.success == false)
    {
        return;
    }

    printf("Binding");
    success &= success == true ? AttemptBind(parsingContext.plan, &optimizeArena) : success;
    printf(" -- %s\n", success == true ? "success" : "failure");
    if (success == false)
    {
        return;
    }
    printf("Heuristics\n");

    String s = S("");
    s = StringifyExpression(s, parsingContext.plan->selection->condition, &optimizeArena);
    printf("\tPre Heuristics WHERE - '%.*s'\n", (int)s.length, s.data);

    ApplyHeuristics(parsingContext.plan, &optimizeArena);

    if (parsingContext.plan->selection->condition != NULL)
    {
        //  If we pushed down all the predicates, this will be empty
        s = S("");
        s = StringifyExpression(s, parsingContext.plan->selection->condition, &optimizeArena);
        printf("\tPost Heuristics WHERE - '%.*s'\n", (int)s.length, s.data);
    }
    else
    {
        printf("\tPost Heuristics WHERE - '()'\n");
    }

    LogicalScan *scan = parsingContext.plan->scans;

    while (scan != NULL)
    {
        printf("Scan '%.*s'\n", (int)scan->name.length, scan->name.data);

        if (scan->filter != NULL)
        {
            String filter = S("");
            filter = StringifyExpression(filter, (Expression *)scan->filter, &optimizeArena);
            printf(
                "\tFilter - '%.*s'\n",
                (int)filter.length,
                filter.data);
        }

        if (scan->scanArguments.length > 0)
        {
            printf("\tArguments - ");

            for (int i = 0; i < scan->scanArguments.length; i++)
            {
                Attribute *attribute = scan->scanArguments.data[i];
                printf(
                    "'%.*s' ",
                    (int)attribute->name.length,
                    attribute->name.data);
            }

            printf("\n");
        }
        scan = scan->next;
    }

    printf("\nFinished\n");
}

void RunTests(Arena *executionArena)
{
    char* sqls[] = 
    {
        "SELECT p.name FROM person p Where p.name = 'joe' and name = 'mike' p.age = 15",
        "SELECT person.name FROM person, place Where name = 'joe' and name = 'mike' and place.id = 100 and person.age = 15",
        "SELECT person.name FROM person, place Where name = 'joe'",
        "SELECT person.name FROM person, place Where name = 'joe' and (person.address_id = 1 or name = 'mike')",
        "SELECT person.name FROM person, place Where person.address_id = place.id and (name = 'joe' or name = 'mike')",
        "SELECT person.name FROM person, place Where name = 'joe' and person.address_id = place.id",
        "SELECT person.name FROM person, place Where age = 1 and (person.name = 'joe' or person.name='mike') and person.address_id = place.id and age = 2",
    };
    
    for (size_t i = 0; i < 1; i++)
    {
        printf("\nTest %ld\n", i);
        Optimize(sqls[i], *executionArena);
    }
}

int main(int argc, char **argv)
{
    printf("SystemR\n");
    char *sql;

    Arena executionArena = NewArena(EXECUTION_ARENA_SIZE);
    BuildCatalog(&executionArena);

    if (argc != 2)
    {
        
        RunTests(&executionArena);
    }
    else
    {
        sql = argv[1];
        Optimize(sql, executionArena);
    }

    free(executionArena.original);
    executionArena.begin = NULL;
    executionArena.end = NULL;

    return 0;
}
