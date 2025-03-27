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
    int next;
};

struct Relation
{
    const char *name;
    int indexCount;
    Attribute attributes[MAX_ARRAY_SIZE];
    bool free;
};

typedef struct
{
    Relation relations[MAX_ARRAY_SIZE];
    Index indices[MAX_ARRAY_SIZE];
    Attribute attributes[MAX_ARRAY_SIZE];
    int relationCount;
} Catalog;

void BuildCatalog(void);
Attribute *GetAttribute(const char *relation, const char *attribute, int *count);

#endif
