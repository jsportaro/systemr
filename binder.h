#ifndef __SYSTEMR_BINDER_H__
#define __SYSTEMR_BINDER_H__

#include <arena.h>
#include <catalog.h>
#include <plan.h>
#include <common.h>
#include <expressions.h>

struct Plan;

typedef enum
{
    BIND_UNBOUND,
    BIND_SUCCESS,
    BIND_NOT_FOUND,
    BIND_CONDITION_NOT_MET,
    BIND_DUPLICATE,
} BindingResult;

typedef struct RelationBinding RelationBinding;

struct RelationBinding
{
    //TableReference *tableReference;
    Relation *boundRelation;
    BindingResult bindingResult;
};

typedef struct
{
    Identifier *identifier;
    Attribute *boundAttribute;
    BindingResult bindingResult;
} AttributeBinding;

bool AttemptBind(Plan *plan, Arena *executionArena);

#endif
