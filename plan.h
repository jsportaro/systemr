#ifndef __SYSTEMR_PLAN_H__
#define __SYSTEMR_PLAN_H__

#include <catalog.h>
#include <sql.h>

typedef enum
{
    PLAN_JOIN,
    LPLAN_SELECT,
    LPLAN_PROJECT,
} PlanNodeType;

typedef struct
{
    PlanNodeType type;
} PlanNode;

typedef struct
{
    PlanNode root;
} Plan;

typedef struct
{
    PlanNodeType type;

    Attribute *left;
    Attribute *right;

    Expression *joinBy;
} LogicalJoin;

#endif
