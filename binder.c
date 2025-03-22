#include <binder.h>
#include <catalog.h>
#include <parser.h>
#include <plan.h>

#include <string.h>

static bool CanBindAttribute(Identifier *unresolved, Relation *relation)
{
    for (int j = 0; j < relation->attributeCount; j++)
    {
        const char *attributeName = relation->attributes[j].name;
        const char *relationName = relation->name;
        
        bool attributeMatch = strncmp(unresolved->name, attributeName, strlen(unresolved->name)) == 0;
        bool relationMatch = 
            (unresolved->qualifier == NULL) || 
            (strncmp(unresolved->qualifier, relationName, strlen(unresolved->name)) == 0);

        if ((attributeMatch == true) &&
            (relationMatch == true))
        {
            

            return true;
        }
    }
    
    return false;
}

Plan *AttemptBind(SelectStatement *selectStatment, Identifier *unresolved, Arena executionArena)
{

}

void AttemptBind(ParsingContext *parsingContext)
{
    RelationBinding relationBindings[MAX_ARRAY_SIZE * 2] = {0};
    
    for (int i = 0; i < parsingContext->selectStatment.tableCount; i++)
    {
        relationBindings[i].tableReference = &parsingContext->selectStatment.tables[i];

        bool found = FindRelation(relationBindings[i].tableReference->identifier.name, &relationBindings[i].boundRelation);
            
        relationBindings[i].bindingResult = found == true ? BIND_SUCCESS : BIND_NOT_FOUND;

        //  Now that we have a table, let's see if they're any columns that match
        Identifier **c = &parsingContext->unresolved;
        while (*c != NULL)
        {
            if (CanBindAttribute(*c, relationBindings[i].boundRelation))
            {
                *c = (*c)->next;
                continue;
            }
            c = &(*c)->next;
        }
    }
}
