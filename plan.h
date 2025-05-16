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
typedef struct Projection Projection;
typedef struct Selection Selection;
typedef struct Join Join;
typedef struct Scan Scan;
typedef struct LogicalScanLookup LogicalScanLookup;

typedef struct
{
   Projection *first;
   Projection *last;
} Projections;

typedef struct Plan
{
    Projections *projections;
    Selection *selection;
    Scan *scans;

    LogicalScanLookup *scansLookup;
} Plan;

struct LogicalRename
{
    const char *name;
};

struct Projection
{
    PlanNodeType type;
    PlanNode *child;

    Expression *projected;
    
    Identifier *identifiers;
    Identifier *unresolved;
};

struct Selection
{
    PlanNodeType type;
    PlanNode *child;
    bool attemptPushdown;
    
    Expression *condition;
    Identifier *unresolved;
};

struct Join
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

struct Scan
{
    PlanNodeType type;

    String name;
    String alias;

    Relation *relation;
    ScanArgumentLookup *argumentsLookup;
    ScanArguments arguments;
    InfixExpression *filter;

    Scan *next;
};

struct LogicalScanLookup
{
    LogicalScanLookup *child[4];
    
    int relationId;
    Scan *scan;
};

Scan *ScanLookup(LogicalScanLookup **scansLookup, int relationId);

#endif
