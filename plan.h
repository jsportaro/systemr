#ifndef __SYSTEMR_PLAN_H__
#define __SYSTEMR_PLAN_H__

#include <catalog.h>
#include <expressions.h>
#include <rstrings.h>

typedef struct Plan Plan;
typedef enum PlanNodeType PlanNodeType;
typedef struct PlanNode PlanNode;
typedef struct Projections Projections;
typedef struct LogicalRename LogicalRename;
typedef struct Projection Projection;
typedef struct Selection Selection;
typedef struct Join Join;
typedef struct Scan Scan;
typedef struct ScanLookup ScanLookup;
typedef struct ScanArguments ScanArguments;
typedef struct ScanArgumentLookup ScanArgumentLookup;

enum PlanNodeType
{
    LPLAN_SELECT,
    LPLAN_PROJECT,
    LPLAN_PROJECT_ALL,
    LPLAN_JOIN,
    LPLAN_SCAN,
};

struct PlanNode
{
    PlanNodeType type;
};

struct Projections
{
   Projection *first;
   Projection *last;
};

struct Plan
{
    Projections *projections;
    Selection *selection;
    Scan *scans;

    ScanLookup *scansLookup;
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

struct ScanLookup
{
    ScanLookup *child[4];
    
    int relationId;
    Scan *scan;
};

Scan *GetScan(ScanLookup **scansLookup, int relationId);

#endif
