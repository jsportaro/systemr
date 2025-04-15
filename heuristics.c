#include <heuristics.h>

static void PushDownProjection(LogicalProjection *projection, Plan *plan, Arena *arena)
{

}

bool ApplyHeuristics(Plan *plan, Arena *arena)
{
    UNUSED(plan);
    UNUSED(arena);

    PushDownProjection(plan->projections->first, plan, arena);
}