#include <arena.h>
#include <common.h>
#include <catalog.h>

#include <string.h>

static Catalog catalog = {0};

static int AddAttribute(int relationId, const char *name, AttributeType type)
{
    //  Assumes there will be room
    int i = Hash(name, strlen(name)) % MAX_ARRAY_SIZE;
    Attribute *attribute = NULL;
    printf("%s %d\n", name, i);
    for (;;)
    {
        attribute = &catalog.attributes[i];
        
        if (attribute->nameLength == 0)
        {
            //  This is empty so break out and assign
            break;
        }

        i = (i + 1) % MAX_ARRAY_SIZE;

        attribute->next = i;
    }
    
    attribute->nameLength = strlen(name);
    attribute->name = name;
    attribute->relationId = relationId;
    attribute->type = type;
    attribute->next = -1;

    return i;
}

Attribute *GetAttribute(const char *relation, const char *attribute, int *count)
{
    int i = Hash(attribute, strlen(attribute)) % MAX_ARRAY_SIZE;
    Attribute *current = NULL;
    Attribute *candidate = NULL;
    for (;;)
    {
        current = &catalog.attributes[i];
        
        if (current->nameLength == 0)
        {
            //  Doesn't exist
            *count = 0;
            return NULL;
        }

        if ((strncmp(attribute, current->name, current->nameLength) == 0) && 
            ((relation == NULL || strncmp(relation, current->name, current->nameLength) == 0) ))
        {
            *count += 1;
            candidate = current;
        }

        i = current->next;

        if (i < 0)
        {
            return candidate;
        }
    }
}

static int AddRelation(const char *name)
{
    int i = Hash(name, strlen(name)) % MAX_ARRAY_SIZE;

    catalog.relations[i].name = name;

    return i;
}

void BuildCatalog(void)
{
    int relation = 0;

    relation = AddRelation("person");
    AddAttribute(relation, "name", ATTR_CHAR);
    AddAttribute(relation, "age", ATTR_INT);
    AddAttribute(relation, "address_id", ATTR_INT);
    AddAttribute(relation, "id", ATTR_INT);
    
    relation = AddRelation("place");
    AddAttribute(relation, "line_one", ATTR_CHAR);
    AddAttribute(relation, "city", ATTR_CHAR);
    AddAttribute(relation, "state", ATTR_CHAR);
    AddAttribute(relation, "zip", ATTR_INT);
    AddAttribute(relation, "id", ATTR_INT);
}
