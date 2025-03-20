#include <binder.h>
#include <catalog.h>
#include <parser.h>

void AttemptBind(ParsingContext *parsingContext)
{
    int found = 0;
    while (parsingContext->unresolved != NULL) 
    {
        switch (parsingContext->unresolved->type)
        {
            case ID_COLUMN:
                FindAttribute(parsingContext->unresolved->name, parsingContext->unresolved->qualifier, &found);
                break;
        }

        parsingContext->unresolved = parsingContext->unresolved->next;
    }

    UNUSED(parsingContext);
}
