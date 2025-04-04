#include <binder.h>
#include <catalog.h>
#include <parser.h>

#include <string.h>

static int BindTableReferences(TableReferenceList *tables, RelationBinding *relationBindings, RelationBinding **aliasLookup)
{
    int found = 0;
    for (int i = 0; i < tables->count; i++)
    {
        TableReference *tableReference = tables->tableReferences[i];
        Relation *relation = GetRelation(tableReference->name);

        if (relation == NULL)
        {
            // Table not found - do this better
            abort();
        }

        relationBindings[found].tableReference = tableReference;
        relationBindings[found].boundRelation = relation;
        size_t aliasLength = strlen(tableReference->alias);
        int i = Hash(tableReference->alias, aliasLength) % MAX_HASH_SIZE;
        for (;;)
        {
            if (aliasLookup[i] == NULL)
            {
                aliasLookup[i] = &relationBindings[found++];

                break;
            }
            else if (strncmp(relationBindings[found].tableReference->alias, aliasLookup[i]->tableReference->alias, aliasLength) == 0)
            {
                // Already have an alias by this name
                abort();
            }

            i = (i + 1) % MAX_HASH_SIZE;
        }
    }

    return found;
}

static RelationBinding *GetRelationBinding(const char* alias, RelationBinding **aliasLookup)
{
    size_t length = strlen(alias);
    uint32_t i = Hash(alias, length) % MAX_HASH_SIZE;

    for (;;)
    {
        //  We should never be unable to find an alias because by now we've
        //  1) Made sure all the aliased projections exist in the FROM clause
        //  2) Made sure all the tables in the FROM clause are in the catalog
        if (aliasLookup[i] == NULL)
        {
            return NULL;
        }
        else if (strncmp(alias, aliasLookup[i]->boundRelation->name, length) == 0)
        {
            return aliasLookup[i];
        }

        i = (i + 1) % MAX_HASH_SIZE;
    }
}

static Attribute *GetAttribute(const char *name, RelationBinding *relationBinding)
{
    size_t length = strlen(name);
    uint32_t i = Hash(name, length) % MAX_HASH_SIZE;

    for (;;)
    {
        if (relationBinding->boundRelation->attributes[i].nameLength == 0)
        {
            return NULL;
        }
        else if (strncmp(name, relationBinding->boundRelation->attributes[i].name, length) == 0)
        {
            return &relationBinding->boundRelation->attributes[i];
        }

        i = (i + 1) % MAX_HASH_SIZE;
    }
}

static void AttemptBindWithAlias(Identifier *unresolved, AttributeBinding *attributeBinding, RelationBinding **aliasLookup)
{
    if (unresolved->qualifier == NULL || attributeBinding->bindingResult != BIND_UNBOUND)
    {
        return;
    }

    RelationBinding *relationBinding = GetRelationBinding(unresolved->qualifier, aliasLookup);
    Attribute *attribute = GetAttribute(unresolved->name, relationBinding);

    if (attribute == NULL)
    {
        attributeBinding->bindingResult = BIND_NOT_FOUND;
    }

    attributeBinding->bindingResult = BIND_SUCCESS;
    attributeBinding->boundAttribute = attribute;
    attributeBinding->identifier = unresolved;
}

static void AttemptBindAnyTable(Identifier *unresolved, AttributeBinding *attributeBinding, RelationBinding *relationBindings, int relationCount)
{
    if (attributeBinding->bindingResult != BIND_UNBOUND)
    {
        return;
    }

    for (int i = 0; i < relationCount; i++)
    {
        RelationBinding *relationBinding = &relationBindings[i];
        Attribute *attribute = GetAttribute(unresolved->name, relationBinding);

        if (attribute == NULL)
        {
            attributeBinding->bindingResult = BIND_NOT_FOUND;
            abort();
        }
        else if (attributeBinding->bindingResult == BIND_SUCCESS)
        {
            //  Ambiguous condition
            abort();
        }

        attributeBinding->bindingResult = BIND_SUCCESS;
        attributeBinding->boundAttribute = attribute;
        attributeBinding->identifier = unresolved;
    }
}

static Plan *BindPlanProjections(SelectExpressionList *selectExpressionList, RelationBinding *relationBindings, RelationBinding **aliasLookup, int relationCount)
{
    bool success = true;
    for (int i = 0; i < selectExpressionList->selectListCount; i++)
    {
        SelectExpression *selectExpression = selectExpressionList->selectList[i];
        Identifier **unresolved = &selectExpression->unresolved;

        while (*unresolved != NULL)
        {
            AttributeBinding attributeBinding = { 0 };

            AttemptBindWithAlias(*unresolved, &attributeBinding, aliasLookup);
            AttemptBindAnyTable(*unresolved, &attributeBinding, relationBindings, relationCount);
            
            if (attributeBinding.bindingResult == BIND_SUCCESS)
            {
                success &= true;

                *unresolved = (*unresolved)->next;
                continue;
            }

            unresolved = &(*unresolved)->next;
        }
    }

    return NULL;
}

Plan *AttemptBind(SelectStatement *selectStatment, Identifier *unresolved, Arena executionArena)
{
    
    Attribute *attributes[MAX_HASH_SIZE] = { 0 };
    RelationBinding relationBindings[MAX_HASH_SIZE] = { 0 };
    RelationBinding *aliasLookup[MAX_HASH_SIZE] = { 0 };

    int relationCount = BindTableReferences(selectStatment->tableReferenceList, relationBindings, aliasLookup);
    Plan *projection = BindPlanProjections(selectStatment->selectExpressionList, relationBindings, aliasLookup, relationCount);

    // Attribute *attribute = NULL;
    // int count = 0;

    // for (int i = 0; i < selectStatment->selectExpressionList->selectListCount; i++)
    // {
    //     SelectExpression *expression = selectStatment->selectExpressionList->selectList[i];

    //     Identifier **c = &expression->unresolved;
    //     while (*c != NULL)
    //     {
    //         attribute = GetAttribute((*c)->qualifier, (*c)->name, &count);
    //         UNUSED(attribute);
    //         if (count == 1)
    //         {
    //             *c = (*c)->next;
    //             continue;
    //         }
    //         c = &(*c)->next;
    //     }
    // }

    UNUSED(selectStatment);
    UNUSED(unresolved);
    UNUSED(executionArena);

    return NULL;
}

// void AttemptBind(ParsingContext *parsingContext)
// {
//     RelationBinding relationBindings[MAX_ARRAY_SIZE * 2] = {0};
    
//     for (int i = 0; i < parsingContext->selectStatement->tableReferenceList->count; i++)
//     {
//         relationBindings[i].tableReference = parsingContext->selectStatement->tableReferenceList->tableReferences[i];

//         bool found = FindRelation(relationBindings[i].tableReference->name, &relationBindings[i].boundRelation);
            
//         relationBindings[i].bindingResult = found == true ? BIND_SUCCESS : BIND_NOT_FOUND;

//         //  Now that we have a table, let's see if they're any columns that match
//         Identifier **c = &parsingContext->unresolved;
//         while (*c != NULL)
//         {
//             if (CanBindAttribute(*c, relationBindings[i].boundRelation))
//             {
//                 *c = (*c)->next;
//                 continue;
//             }
//             c = &(*c)->next;
//         }
//     }
// }
