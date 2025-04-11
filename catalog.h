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

struct Index
{
    const char *columns[MAX_ARRAY_SIZE];
    int columnCount;
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
    String name;
    Attributes *attributes;
};

void BuildCatalog(Arena *arena);
Relation *GetRelation(String relation);

#endif
