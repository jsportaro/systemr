#include <catalog.h>
#include <common.h>
#include <binder.h>
#include <parser.h>
#include <plan.h>

#include <stdio.h>
#include <string.h>

static void PrettyPrintPlan(PlanNode *c)
{


    if (c == NULL)
    {
        return;
    }

    switch (c->type)
    {
        case LPLAN_PROJECT: {
            LogicalProjection *project = (LogicalProjection *)c;
            printf("    |\n");
            printf("(Project)\n");
            PrettyPrintPlan(project->child);
            break;
        }
        case LPLAN_SELECT: {
            LogicalSelection *select = (LogicalSelection *)c;
            printf("    |\n");
            printf("Selection\n");
            PrettyPrintPlan(select->child);
            break;
        }
        case LPLAN_JOIN: {
            LogicalJoin *join = (LogicalJoin *)c;
            printf("    |\n");
            printf("Join\n");
            printf(" |   \\\n");
            PrettyPrintPlan(join->left);
            PrettyPrintPlan(join->right);
            printf("\n");
            break;
        }
        case LPLAN_SCAN: {
            LogicalScan *scan = (LogicalScan*)c;
            printf(" Scan %s", scan->name);
            break;
        }
    }
}

int main(void)
{
    printf("SystemR\n");
    //char *sql = "SELECT person.name AS FullName, place.city AS Town, zip as Zip FROM person, place, thing WHERE person.address_id = place.id;";
    char *sql = "SELECT person.name, name, a.name, * FROM person;";
    //char *sql = "SELECT p.name, age FROM person p WHERE p.name = 'joe';";

    BuildCatalog();
    Arena executionArena = NewArena(EXECUTION_ARENA_SIZE);
    ParsingContext parsingContext = { 0 };
    parsingContext.parseArena = &executionArena;

    ParseSQL(&parsingContext, sql, strlen(sql));

    printf("Parsing %s\n", parsingContext.success == true ? "success" : "failure");

    //Plan *plan = AttemptBind(parserContext.selectStatement, &executionArena);

    //PrettyPrintPlan(parsingContext.plan->root);

    free(executionArena.original);
    executionArena.begin = NULL;
    executionArena.end = NULL;

    return 0;
}
