#include <binder.h>
#include <catalog.h>
#include <parser.h>

#include <string.h>

void AttemptBind(ParsingContext *parsingContext)
{
    RelationBinding relationBindings[MAX_ARRAY_SIZE * 2] = {0};

    int boundRelations = 0;

    for (int i = 0; i < parsingContext->selectStatment.tableCount; i++)
    {
        relationBindings[i].tableReference = &parsingContext->selectStatment.tables[i];

        bool found = FindRelation(relationBindings[i].tableReference->identifier.name, &relationBindings[i].boundRelation);
            
        relationBindings[i].bindingResult = found == true ? BIND_SUCCESS : BIND_NOT_FOUND;

        //  Now that we have a table, let's see if they're any columns that match
        Identifier *current = parsingContext->unresolved;
        while (current != NULL)
        {
            for (int j = 0; j < relationBindings[i].boundRelation->attributeCount; j++)
            {
                const char *attributeName = relationBindings[i].boundRelation->attributes[j].name;
                const char *relationName = relationBindings[i].tableReference->identifier.name;
                bool attributeMatch = strncmp(current->name, attributeName, strlen(current->name)) == 0;
                bool relationMatch = (current->qualifier == NULL) || (strncmp(current->qualifier, relationName, strlen(current->name)) == 0);
                if ((attributeMatch == true) &&
                    (relationMatch == true))
                {
                    int asg = 0;
                }
            }

            current = current->next;
        }

    }
}
