#include <arena.h>
#include <common.h>
#include <catalog.h>

#include <string.h>

static Catalog catalog = {0};

static int AddAttribute(int relationId, const char *name, AttributeType type)
{
    size_t nameLength = strlen(name);
    int i = Hash(name, nameLength) % MAX_HASH_SIZE;
    Relation *relation = &catalog.relations[relationId];
    Attribute *attribute = NULL;
    
     for (;;)
    {
        attribute = &relation->attributes[i];
        
        if (attribute->nameLength == 0)
        {
            //  This is empty so break out and assign
            break;
        }

        i = (i + 1) % MAX_HASH_SIZE;
    }

    attribute->nameLength = strlen(name);
    attribute->name = name;
    attribute->relationId = relationId;
    attribute->type = type;
    attribute->relation = relation;

    catalog.attributes[catalog.attributeCount++] = attribute;

    return i;
}

static int AddRelation(const char *name)
{
    size_t length = strlen(name);
    uint32_t i = Hash(name, length) % MAX_HASH_SIZE;

    //  If neither name exists, we're good to add.
    //  However, if a reference already exists in either case then we have a duplicate name collision
    for (;;)
    {
        if (catalog.relations[i].isSet == false)
        {
            catalog.relations[i].isSet = true;
            catalog.relations[i].name = name;

            return i;
        }
        else if ((catalog.relations[i].isSet = true) && (strncmp(name, catalog.relations[i].name, length) == 0))
        {
            //  Alias already exists
            return i;
        }

        i = (i + 1) % MAX_HASH_SIZE;
    }
}

Relation *GetRelation(const char *relation)
{
    size_t length = strlen(relation);
    uint32_t i = Hash(relation, length) % MAX_HASH_SIZE;

    for (;;)
    {
        if (catalog.relations[i].isSet == false)
        {
            return NULL;
        }
        else if (strncmp(relation, catalog.relations[i].name, length) == 0)
        {
            //  Alias already exists
            return &catalog.relations[i];
        }

        i = (i + 1) % MAX_HASH_SIZE;
    }
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
