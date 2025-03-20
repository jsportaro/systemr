#ifndef __SYSTEMR_BINDER_H__
#define __SYSTEMR_BINDER_H__

#include <catalog.h>
#include <common.h>
#include <sql.h>

typedef struct
{
    TableReference *tableReference;
    Relation *boundRelation;
} RelationBinding;


void AttemptBind(ParsingContext *parsingContext);

#endif
