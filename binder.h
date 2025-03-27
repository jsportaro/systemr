#ifndef __SYSTEMR_BINDER_H__
#define __SYSTEMR_BINDER_H__

#include <catalog.h>
#include <common.h>
#include <plan.h>
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
    Identifier *identifier;
    Attribute *boundAttribute;
    BindingResult bindingResult;
    RelationBinding *relation;
} AttributeBinding;

Plan *AttemptBind(SelectStatement *selectStatment, Identifier *unresolved, Arena executionArena);

#endif
