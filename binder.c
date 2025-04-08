#include <binder.h>
#include <catalog.h>
#include <parser.h>
#include <plan.h>

#include <string.h>

typedef struct
{
    SelectStatement *selectStatment;
    Arena *executionArena;

    RelationBinding relationBindings[MAX_ARRAY_SIZE];
    int relationBindingsCount;

    AttributeBinding attributeBindings[MAX_ARRAY_SIZE];
    int attributeBindingCount;

    RelationBinding *aliasLookup[MAX_HASH_SIZE];
    AttributeBinding *attributeBindingLookup[MAX_HASH_SIZE];
} BindingContext;

static AttributeBinding *GetOrAddAttributeBinding(AttributeBinding **attributeBindings, AttributeBinding *binding)
{
    int i = binding->boundAttribute->hash % MAX_HASH_SIZE;

    for (;;)
    {
        if (attributeBindings[i] == NULL)
        {
            attributeBindings[i] = binding;
        }

        i = (i + 1) % MAX_HASH_SIZE;
    }

    return NULL;
}

static void BindTableReferences(BindingContext *bindingContext)
{
    int found = 0;
    TableReferenceList *tables = bindingContext->selectStatment->tableReferenceList;
    RelationBinding *relationBindings = bindingContext->relationBindings;
    RelationBinding **aliasLookup = bindingContext->aliasLookup;

    for (int i = 0; i < tables->count; i++)
    {
        TableReference *tableReference = tables->tableReferences[i];
        Relation *relation = GetRelation(tableReference->name);

        if (relation == NULL)
        {
            // Table not found - do this better
            abort();
        }

        //relationBindings[found].tableReference = tableReference;
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
            // else if (strncmp(relationBindings[found].tableReference->alias, aliasLookup[i]->tableReference->alias, aliasLength) == 0)
            // {
            //     // Already have an alias by this name
            //     abort();
            // }

            i = (i + 1) % MAX_HASH_SIZE;
        }
    }

    bindingContext->relationBindingsCount = found;
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

static void AttemptBindWithAlias(BindingContext *bindingContext, Identifier *unresolved, AttributeBinding *attributeBinding)
{
    RelationBinding **aliasLookup = bindingContext->aliasLookup;
    
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

    //  Add to hash
}

static void AttemptBindAnyTable(BindingContext *bindingContext, Identifier *unresolved, AttributeBinding *attributeBinding)
{
    RelationBinding *relationBindings = bindingContext->relationBindings;
    int relationBindingsCount = bindingContext->relationBindingsCount;

    if (attributeBinding->bindingResult != BIND_UNBOUND)
    {
        return;
    }

    attributeBinding->bindingResult = BIND_NOT_FOUND;

    for (int i = 0; i < relationBindingsCount; i++)
    {
        RelationBinding *relationBinding = &relationBindings[i];
        Attribute *attribute = GetAttribute(unresolved->name, relationBinding);

        if (attribute != NULL)
        {
            attributeBinding->identifier = unresolved;

            if (attributeBinding->bindingResult == BIND_SUCCESS)
            {
                // duplicate conditions
                attributeBinding->bindingResult = BIND_DUPLICATE;
                attributeBinding->boundAttribute = NULL;
            }
            else
            {
                attributeBinding->bindingResult = BIND_SUCCESS;
                attributeBinding->boundAttribute = attribute;
            }
        }
    }
}

static Projection *AddProjection(BindingContext *BindingContext, Projection *parent, AttributeBinding *attributeBinding)
{
    Projection *projection = NEW(BindingContext->executionArena, Projection);

    projection->type = LPLAN_PROJECT;
    //projection->attributeBinding = attributeBinding;

    if (parent != NULL)
    {
        parent->child = projection;
    }

    return projection;
}

#define WHEN_UNBOUND(bindingMethod)                         \
do {                                                        \
    if (attributeBinding->bindingResult == BIND_UNBOUND)    \
    {                                                       \
        (bindingMethod);                                    \
    }                                                       \
} while(false)

static Projection *BindProjections(BindingContext *bindingContext, Plan *plan)
{
    Projection *lastAdded = NULL;

    SelectExpressionList *selectExpressionList = bindingContext->selectStatment->selectExpressionList;
    AttributeBinding **attributeBindings = bindingContext->attributeBindings;

    for (int i = 0; i < selectExpressionList->selectListCount; i++)
    {
        SelectExpression *selectExpression = selectExpressionList->selectList[i];
        Identifier **unresolved = &selectExpression->unresolved;

        while (*unresolved != NULL)
        {
            AttributeBinding *attributeBinding = &bindingContext->attributeBindings[bindingContext->attributeBindingCount++];

            WHEN_UNBOUND(AttemptBindWithAlias(bindingContext, *unresolved, attributeBinding));
            WHEN_UNBOUND(AttemptBindAnyTable(bindingContext, *unresolved, attributeBinding));
            
            //  Need to handle error case
            if (attributeBinding->bindingResult == BIND_SUCCESS)
            {
                lastAdded = AddProjection(bindingContext, lastAdded, attributeBinding);

                if (plan->root == NULL)
                {
                    plan->root = lastAdded;
                }
            }
            else
            {
                abort();
            }
            *unresolved = (*unresolved)->next;
        }
    }

    return lastAdded;
}

static Selection *BindSelection(BindingContext *bindingContext)
{
    Selection *selection = NEW(bindingContext->executionArena, Selection);

    Identifier **unresolved = &bindingContext->selectStatment->whereExpression->unresolved;
    
    while (*unresolved != NULL)
    {
        AttributeBinding *attributeBinding = &bindingContext->attributeBindings[bindingContext->attributeBindingCount++];

        WHEN_UNBOUND(AttemptBindWithAlias(bindingContext, *unresolved, attributeBinding));
        WHEN_UNBOUND(AttemptBindAnyTable(bindingContext, *unresolved, attributeBinding));
        
        //  Need to handle error case
        if (attributeBinding->bindingResult == BIND_SUCCESS)
        {
            
        }
        else
        {
            abort();
        }
        *unresolved = (*unresolved)->next;
    }

    return selection;
}

#undef WHEN_UNBOUND



struct Plan *AttemptBind(SelectStatement *selectStatement, Arena *executionArena)
{
    BindingContext *bindingContext = NEW(executionArena, BindingContext);
    Plan *plan = NEW(executionArena, Plan);
    bindingContext->executionArena = executionArena;
    bindingContext->selectStatment = selectStatement;
    BindTableReferences(bindingContext);
    
    Projection *projection = BindProjections(bindingContext, plan);
    projection->child = BindSelection(bindingContext);

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

    return plan;
}
