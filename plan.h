#ifndef __SYSTEMR_PLAN_H__
#define __SYSTEMR_PLAN_H__

#include <catalog.h>
#include <expressions.h>
#include <rstrings.h>

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

typedef struct LogicalRename LogicalRename;
typedef struct LogicalProjection LogicalProjection;
typedef struct LogicalSelection LogicalSelection;
typedef struct LogicalJoin LogicalJoin;
typedef struct LogicalScan LogicalScan;

typedef struct
{
   LogicalProjection *first;
   LogicalProjection *last;
} LogicalProjections;

typedef struct Plan
{
    LogicalProjections *projections;
    LogicalSelection *selection;
    LogicalScan *scans;
} Plan;

struct LogicalRename
{
    const char *name;
};

struct LogicalProjection
{
    PlanNodeType type;
    PlanNode *child;

    Expression *projected;
    
    Identifier *identifiers;
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

typedef struct ScanArgument ScanArgument;

struct ScanArgument
{
    Attribute *attribute;
};

struct LogicalScan
{
    PlanNodeType type;

    String name;
    String alias;

    Relation *relation;
    
    LogicalScan *next;
};

#endif
