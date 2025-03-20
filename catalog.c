#include <arena.h>
#include <catalog.h>

#include <string.h>

static Catalog catalog;

static int AddAttribute(int relation, const char *name, AttributeType type)
{
    int i = catalog.relations[relation].attributeCount++;

    catalog.relations[relation].attributes[i].name = name;
    catalog.relations[relation].attributes[i].type = type;

    return i;
}

static int AddRelation(const char *name)
{
    int i = catalog.relationCount++;

    catalog.relations[i].name = name;

    return i;
}

static int NoCmp(const char *s1, const char *s2, size_t n)
{
    UNUSED(s1);
    UNUSED(s2);
    UNUSED(n);

    return 0;
}

bool FindRelation(const char *relation, Relation **found)
{
    size_t relationLength = strlen(relation);
    *found = NULL;

    for (int i = 0; i < catalog.relationCount; i++)
    {
        if (strncmp(catalog.relations[i].name, relation, relationLength) == 0)
        {
            *found = &catalog.relations[i];
        }
    }

    return *found != NULL;
}

Attribute *FindAttribute(const char *attribute, const char *relation, int *found)
{
    size_t relationLength = strlen(relation);
    size_t attributeLength = strlen(relation);
    Attribute *attr = NULL;
    int (*cmp)(const char *s1, const char *s2, size_t n);

    cmp = relation == NULL ? &NoCmp : &strncmp;

    for (int i = 0; i < catalog.relationCount; i++)
    {
        if (cmp(catalog.relations[i].name, relation, relationLength) == 0)
        {
            for (int j = 0; j < catalog.relations[i].attributeCount; j++)
            {
                if (strncmp(catalog.relations[i].attributes[j].name, attribute, attributeLength) == 0)
                {
                    *found += 1;
                    attr = &catalog.relations[i].attributes[j];
                }
            }
        }
    }

    return attr;
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
    AddAttribute(relation, "id", ATTR_INT);
}

