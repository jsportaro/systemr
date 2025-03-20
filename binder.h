#ifndef __SYSTEMR_BINDER_H__
#define __SYSTEMR_BINDER_H__

#include <catalog.h>
#include <common.h>
#include <sql.h>

typedef enum
{
    BIND_SUCCESS,
    BIND_NOT_FOUND,
    BIND_DUPLICATE,
} BindingResult;

typedef struct
{
    TableReference *tableReference;
    Relation *boundRelation;
    BindingResult bindingResult;
} RelationBinding;

typedef struct
{
    Identifier *Identifier;
    Attribute *boundAttribute;
    BindingResult bindingResult;
} AttributeBinding;


void AttemptBind(ParsingContext *parsingContext);

#endif
