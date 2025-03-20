#ifndef __SYSTEMR_CATALOG_H__
#define __SYSTEMR_CATALOG_H__

#include <common.h>

typedef enum
{
    ATTR_INT,
    ATTR_CHAR,
} AttributeType;

typedef struct
{
    const char *columns[MAX_ARRAY_SIZE];
    int columnCount;
} Index;

typedef struct
{
    const char *name;
    AttributeType type;
} Attribute;

typedef struct
{
    const char *name;

    Index indices[MAX_ARRAY_SIZE];
    int indexCount;

    Attribute attributes[MAX_ARRAY_SIZE];
    int attributeCount;
} Relation;

typedef struct
{
    Relation relations[MAX_ARRAY_SIZE];
    int relationCount;
} Catalog;

void BuildCatalog(void);
Attribute *FindAttribute(const char *attribute, const char *relation, int *found);
bool FindRelation(const char *relation, Relation **found);

#endif
