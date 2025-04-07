#ifndef __SYSTEMR_PLAN_H__
#define __SYSTEMR_PLAN_H__

#include <catalog.h>
#include <binder.h>
#include <sql.h>

typedef enum
{
    PLAN_ROOT,
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
    PlanNode *root;
} Plan;

typedef struct Selection Selection;
typedef struct Projection Projection;

struct Projection
{
    PlanNodeType type;
    PlanNode *child;

    AttributeBinding *attributeBinding;
};

struct Selection
{
    PlanNodeType type;
    PlanNode *child;

    Expression *condition;
};

typedef struct
{
    PlanNodeType type;

    Attribute *left;
    Attribute *right;

    Expression *joinBy;
} LogicalJoin;

#endif
