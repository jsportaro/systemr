#ifndef __SYSTEMR_CATALOG_H__
#define __SYSTEMR_CATALOG_H__

#include <common.h>
#include <rstrings.h>

typedef enum
{
    ATTR_INT,
    ATTR_CHAR,
} AttributeType;

typedef struct Index Index;
typedef struct Attribute Attribute;
typedef struct Relation Relation;
typedef struct Relations Relations;
typedef struct Attributes Attributes;
typedef struct Indices Indices;

struct Index
{
    Attributes *indexed;
    String name;
};

struct Attribute
{
    int id;
    String name;
    AttributeType type;
    int relationId;
    Relation *relation;
    uint32_t hash;
};

struct Relation
{
    int id;
    String name;
    Attributes *attributes;
    Indices *indices;
};

void BuildCatalog(Arena *arena);
Relation *GetRelation(String relation);
Attribute *GetAttribute(Relation *relation, String name);

#endif
