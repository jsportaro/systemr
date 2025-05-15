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
typedef struct Indexes Indexes;
typedef struct RelationStatistics RelationStatistics;
typedef struct IndexStatistics IndexStatistics;

struct Attribute
{
    int id;
    String name;
    AttributeType type;
    Relation *relation;
    uint32_t hash;
};

struct RelationStatistics
{
    int ncard; // Cardinality of relation
    int tcard; // Number of pages in the relation
};

struct IndexStatistics
{
    int nindx; // Number of pages in the index
    int icard; // Number of distinct keys in index
};

struct Index
{
    Attributes *attributes;
    String name;
    IndexStatistics statistics;
};

struct Relation
{
    int id;
    String name;
    Attributes *attributes;
    Indexes *indexes;

    RelationStatistics statistics;
};

void BuildCatalog(Arena *arena);
Relation *GetRelation(String relation);
Attribute *GetAttribute(Relation *relation, String name);

#endif
