#include <arena.h>
#include <common.h>
#include <catalog.h>
#include <rstrings.h>

#include <stdarg.h>
#include <string.h>

typedef struct
{
    Relations *relations;
    int objects;
} Catalog;

struct Relations
{
    Relations *child[4];
    Relation relation;
};

struct Attributes
{
    Attributes *child[4];
    Attribute attribute;
};

struct Indexes
{
    Indexes *child[4];
    Index index;
};

static Catalog catalog = {0};

static Attribute *LookupAttribute(Attributes **attributes, String name, Arena *arena)
{
    for (uint64_t hash = HashString(name); *attributes; hash <<= 2)
    {
        if (Equals(name, (*attributes)->attribute.name) == true)
        {
            return &(*attributes)->attribute;
        }

        ptrdiff_t i = hash >> 62;
        attributes = &(*attributes)->child[i];
    }

    if (arena == NULL)
    {
        return NULL;
    }

    *attributes = NEW(arena, Attributes);
    (*attributes)->attribute.name = name;

    return &(*attributes)->attribute;
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
        if (Equals(name, (*relations)->relation.name) == true)
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
    (*relations)->relation.name = name;
    
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

static Index *GetOrCreateIndex(String name, Relation *relation, Arena *arena)
{
    //  Also, make sure the index name is globally unique.
    //  That's more of a nice to have.  Nobody is creating
    //  anything with DDL so I can trust the `BuildCatalog`
    //  function to keep things unique.  But, if I expand 
    //  this to something larger - don't forget!

    Indexes **indexes = &relation->indexes;

    for (uint64_t hash = HashString(name); *indexes; hash <<= 2)
    {
        if (Equals(name, (*indexes)->index.name) == true)
        {
            return &(*indexes)->index;
        }

        ptrdiff_t i = hash >> 62;
        indexes = &(*indexes)->child[i];
    }

    *indexes = NEW(arena, Indexes);
    (*indexes)->index.name = name;

    return &(*indexes)->index;
}

static void IndexOnAttribute(Index *index, Attribute *attribute, Arena *arena)
{
    LookupAttribute(&index->attributes, attribute->name, arena);
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
    Attribute *personId = AddAttribute(relation, "id", ATTR_INT, arena);
    Attribute *name = AddAttribute(relation, "name", ATTR_CHAR, arena);
    AddAttribute(relation, "address_id", ATTR_INT, arena);
    AddAttribute(relation, "age", ATTR_INT, arena);

    index = GetOrCreateIndex(S("idx_person_name"), relation, arena);
    IndexOnAttribute(index, name, arena);
    index->statistics.icard = 10;
    index->statistics.nindx = 1;

    index = GetOrCreateIndex(S("idx_person_id"), relation, arena);
    IndexOnAttribute(index, personId, arena);
    index->statistics.icard = 10;
    index->statistics.nindx = 1;

    relation = AddRelation("place", arena);
    Attribute *placeId = AddAttribute(relation, "id", ATTR_INT, arena);
    AddAttribute(relation, "line_one", ATTR_CHAR, arena);
    AddAttribute(relation, "city", ATTR_CHAR, arena);
    AddAttribute(relation, "state", ATTR_CHAR, arena);
    AddAttribute(relation, "zip", ATTR_INT, arena);

    index = GetOrCreateIndex(S("idx_place_id"), relation, arena);
    IndexOnAttribute(index, placeId, arena);
}
