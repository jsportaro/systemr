#include <arena.h>

typedef struct
{
    int a;
    int b;
    int c;
} Thing;

int main(void)
{
    Arena perm = NewArena(500);
    Thing *t1 = NEW(&perm, Thing);
    Thing *t2 = NEW(&perm, Thing);
}