#include <binder.h>
#include <catalog.h>
#include <parser.h>

void AttemptBind(ParsingContext *parsingContext)
{
    RelationBinding relationBindings[MAX_ARRAY_SIZE] = {0};
    bool wasBound = true;
    for (int i = 0; i < parsingContext->selectStatment.tableCount; i++)
    {
        relationBindings[i].tableReference = &parsingContext->selectStatment.tables[i];

        if (FindRelation(relationBindings[i].tableReference->identifier.name, &relationBindings[i].boundRelation) != true)
        {
            wasBound &= false;
        }
    }
}
