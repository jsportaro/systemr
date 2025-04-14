#include <catalog.h>
#include <common.h>
#include <binder.h>
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
        sql = "SELECT person.name FROM person;";
        printf("Demo SQL %sd\n", sql);
    }
    else
    {
        sql = argv[1];
    }

    printf("Optimizing %s\n", sql);
    Arena executionArena = NewArena(EXECUTION_ARENA_SIZE);
    ParsingContext parsingContext = { 0 };

    BuildCatalog(&executionArena);
    parsingContext.parseArena = &executionArena;

    ParseSQL(&parsingContext, sql, strlen(sql));
    printf("Parsing");
    printf(" -- %s\n", parsingContext.success == true ? "success" : "failure");

    if (parsingContext.success == true)
    {
        printf("Binding -- ");

        if (AttemptBind(parsingContext.plan, &executionArena) == true)
        {
            printf("success\n");
        }
        else
        {
            printf("failure\n");
        }
    }

    free(executionArena.original);
    executionArena.begin = NULL;
    executionArena.end = NULL;

    return 0;
}
