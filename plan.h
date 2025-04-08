#ifndef __SYSTEMR_PLAN_H__
#define __SYSTEMR_PLAN_H__

#include <catalog.h>
#include <binder.h>
#include <sql.h>

typedef enum PlanNodeType
{
    PLAN_ROOT,
    PLAN_JOIN,
    LPLAN_SELECT,
    LPLAN_PROJECT,
    LPLAN_JOIN,
    LPLAN_SCAN,
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

typedef struct LogicalProjection LogicalProjection;
typedef struct LogicalSelection LogicalSelection;
typedef struct LogicalJoin LogicalJoin;
typedef struct LogicalScan LogicalScan;

struct Projection
{
    PlanNodeType type;
    PlanNode *child;
};

struct Selection
{
    PlanNodeType type;
    PlanNode *child;

    Expression *condition;
};

struct LogicalProjection
{
    PlanNodeType type;
    PlanNode *child;

    Expression *expression;
    Identifier *unresolved;
};

struct LogicalSelection
{
    PlanNodeType type;
    PlanNode *child;

    Expression *condition;
    Identifier *unresolved;
};

struct LogicalJoin
{
    PlanNodeType type;

    PlanNode *left;
    PlanNode *right;

    Expression *joinBy;
};

struct LogicalScan
{
    PlanNodeType type;

    const char *name;
    const char *alias;

    LogicalScan *next;
};

#endif
