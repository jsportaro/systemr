#ifndef __SYSTEMR_PLAN_H__
#define __SYSTEMR_PLAN_H__

#include <catalog.h>
#include <expressions.h>

typedef enum PlanNodeType
{
    LPLAN_SELECT,
    LPLAN_PROJECT,
    LPLAN_PROJECT_ALL,
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

typedef struct LogicalRename LogicalRename;
typedef struct LogicalProjection LogicalProjection;
typedef struct LogicalSelection LogicalSelection;
typedef struct LogicalJoin LogicalJoin;
typedef struct LogicalScan LogicalScan;

struct LogicalRename
{
    const char *name;
};

struct LogicalProjection
{
    PlanNodeType type;
    PlanNode *child;

    Expression *projected;
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
