#include <catalog.h>
#include <common.h>
#include <binder.h>
#include <parser.h>
#include <plan.h>

#include <stdio.h>
#include <string.h>

static void PrettyPrintPlan(PlanNode **c)
{
    printf("    |\n");


    if (*c == NULL)
    {
        return;
    }

    switch ((*c)->type)
    {
        case LPLAN_PROJECT: {
            Projection *project = (Projection *)*c;
            c = &(project)->child;

            printf("(Project (%s->%s))\n", project->attributeBinding->boundAttribute->relation->name, project->attributeBinding->boundAttribute->name);
            PrettyPrintPlan(c);
            break;
        }
        case LPLAN_SELECT: {
            Selection *project = (Selection *)*c;
            c = &(project)->child;
            PrettyPrintPlan(c);
            break;
        }
    }
}

int main(void)
{
    printf("SystemR\n");
    char *sql = "SELECT person.name AS FullName, place.city AS Town, zip as Zip FROM person, place WHERE person.address_id = place.id;";
    //char *sql = "SELECT person.name, name, a.name FROM person;";
    //char *sql = "SELECT p.name, age FROM person p WHERE p.name = 'joe';";

    BuildCatalog();
    Arena executionArena = NewArena(EXECUTION_ARENA_SIZE);
    ParsingContext parserContext = ParseSQL(sql, strlen(sql), &executionArena);    
    Plan *plan = AttemptBind(parserContext.selectStatement, &executionArena);

    PlanNode **c = &plan->root;
    PrettyPrintPlan(c);

    free(executionArena.original);
    executionArena.begin = NULL;
    executionArena.end = NULL;

    UNUSED(parserContext);

    return 0;
}
