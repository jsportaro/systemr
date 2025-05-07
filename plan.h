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
typedef struct LogicalScanLookup LogicalScanLookup;

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

    LogicalScanLookup *scansLookup;
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
    bool attemptPushdown;
    
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

typedef struct ScanArguments ScanArguments;
typedef struct ScanArgumentLookup ScanArgumentLookup;

struct ScanArguments
{
    Attribute **data;
    ptrdiff_t length;
    ptrdiff_t capacity;
};

struct ScanArgumentLookup
{
    ScanArgumentLookup *child[4];
    
    int attributeId;
};

struct LogicalScan
{
    PlanNodeType type;

    String name;
    String alias;

    Relation *relation;
    ScanArgumentLookup *scanArgumentsLookup;
    ScanArguments scanArguments;
    Expression *filter;

    LogicalScan *next;
};

struct LogicalScanLookup
{
    LogicalScanLookup *child[4];
    
    int relationId;
    LogicalScan *scan;
};

LogicalScan *ScanLookup(LogicalScanLookup **scansLookup, int relationId);

#endif
