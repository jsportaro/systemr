#include <arena.h>
#include <common.h>
#include <plan.h>

Scan *GetScan(ScanLookup **scansLookup, int relationId)
{
    for (uint64_t h = relationId; *scansLookup; h <<= 2)
    {
        if (relationId == (*scansLookup)->relationId)
        {
            return (*scansLookup)->scan;
        }

        scansLookup = &(*scansLookup)->child[h >> 62];
    }

    return NULL; 
}