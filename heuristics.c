#include <heuristics.h>

static void PushDownProjections(Plan *plan, Arena *arena)
{
    LogicalProjection *projection = plan->projections;

    
}

bool ApplyHeuristics(Plan *plan, Arena *arena)
{
    UNUSED(plan);
    UNUSED(arena);

    PushDownProjections(plan, arena);
}