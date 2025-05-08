#include <arena.h>
#include <common.h>
#include <catalog.h>
#include <rstrings.h>

#include <string.h>

typedef struct
{
    Relations *relations;
    int objects;
} Catalog;

struct Relations
{
    Relations *child[4];
    String name;
    Relation relation;
};

struct Attributes
{
    Attributes *child[4];
    String name;
    Attribute relation;
};

static Catalog catalog = {0};

static Attribute *LookupAttribute(Attributes **attributes, String name, Arena *arena)
{
    for (uint64_t hash = HashString(name); *attributes; hash <<= 2)
    {
        if (Equals(name, (*attributes)->name) == true)
        {
            return &(*attributes)->relation;
        }

        ptrdiff_t i = hash >> 62;
        attributes = &(*attributes)->child[i];
    }

    if (arena == NULL)
    {
        return NULL;
    }

    *attributes = NEW(arena, Attributes);
    (*attributes)->name = name;

    return &(*attributes)->relation;
}

static Attribute *AddAttribute(Relation *relation, const char *name, AttributeType type, Arena *arena)
{
    String attributeName =  S(name);
    Attribute *attribute = LookupAttribute(&relation->attributes, attributeName, arena);

    attribute->name = attributeName;
    attribute->type = type;
    attribute->id = catalog.objects++;
    attribute->relation = relation;

    return attribute;
}

static Relation *LookupRelation(String name, Arena *arena)
{
    Relations **relations = &catalog.relations;
    uint64_t hash = 0;
    for (hash = HashString(name); *relations; hash <<= 2)
    {
        if (Equals(name, (*relations)->name) == true)
        {
            return &(*relations)->relation;
        }

        relations = &(*relations)->child[hash >> 62];
    }

    if (arena == NULL)
    {
        return NULL;
    }

    *relations = NEW(arena, Relations);
    (*relations)->name = name;
    
    return &(*relations)->relation;
}

static Relation *AddRelation(const char *name, Arena *arena)
{
    String n = S(name);

    Relation *relation = LookupRelation(n, arena);
    relation->name = n;
    relation->id = catalog.objects++;
    
    return relation;
}

static Index *AddIndex(const char *name, Relation *relation)
{
    return NULL;
}

static void IndexOnAttribute(Index *index, Attribute *attribute)
{

}

Relation *GetRelation(String relation)
{
    return LookupRelation(relation, NULL);
}

Attribute *GetAttribute(Relation *relation, String name)
{
    return LookupAttribute(&relation->attributes, name, NULL);
}

void BuildCatalog(Arena *arena)
{
    Relation *relation = NULL;
    Index *index = NULL;

    relation = AddRelation("person", arena);
    Attribute *name = AddAttribute(relation, "name", ATTR_CHAR, arena);
    AddAttribute(relation, "age", ATTR_INT, arena);
    AddAttribute(relation, "address_id", ATTR_INT, arena);
    Attribute *personId = AddAttribute(relation, "id", ATTR_INT, arena);

    index = AddIndex("idx_name", relation);
    IndexOnAttribute(index, name);

    index = AddIndex("idx_id", relation);
    IndexOnAttribute(index, personId);

    relation = AddRelation("place", arena);
    AddAttribute(relation, "line_one", ATTR_CHAR, arena);
    AddAttribute(relation, "city", ATTR_CHAR, arena);
    AddAttribute(relation, "state", ATTR_CHAR, arena);
    AddAttribute(relation, "zip", ATTR_INT, arena);
    AddAttribute(relation, "id", ATTR_INT, arena);
}
