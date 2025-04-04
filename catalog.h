#ifndef __SYSTEMR_CATALOG_H__
#define __SYSTEMR_CATALOG_H__

#include <common.h>

typedef enum
{
    ATTR_INT,
    ATTR_CHAR,
} AttributeType;

typedef struct Index Index;
typedef struct Attribute Attribute;
typedef struct Relation Relation;

struct Index
{
    const char *columns[MAX_ARRAY_SIZE];
    int columnCount;
};

struct Attribute
{
    const char *name;
    size_t nameLength;
    AttributeType type;
    int relationId;
    Relation *relation;
};

struct Relation
{
    const char *name;
    bool isSet;
    Attribute attributes[MAX_HASH_SIZE];
};

typedef struct
{
    Relation relations[MAX_ARRAY_SIZE];
    int relationCount;

    Index indices[MAX_ARRAY_SIZE];
    int indexCount;

    Attribute *attributes[MAX_ARRAY_SIZE];
    int attributeCount;
} Catalog;

void BuildCatalog(void);
//Attribute *GetAttribute(const char *relation, const char *attribute, int *count);
Relation *GetRelation(const char *relation);

#endif
