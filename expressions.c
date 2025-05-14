#include <arena.h>
#include <common.h>
#include <expressions.h>
#include <rstrings.h>

String NumberToString(int i, Arena *arena)
{
    int maxSize = 15;
    char *stringData = NEW(arena, char, maxSize);
    sprintf(stringData, "%d", i);

    return S(stringData);
}

String StringifyExpression(String string, Expression *expression, Arena *arena)
{
    switch (expression->type)
    {
        case EXPR_IDENIFIER: {
            TermExpression *term = (TermExpression *)expression;
            Identifier *identifier = &term->value.identifier;

            if (identifier->qualifier.length != 0)
            {
                string = Concat(string, identifier->qualifier, arena);
                string = Concat(string, S("."), arena);
            }

            string = Concat(string, identifier->name, arena);

            if (identifier->attribute != NULL)
            {
                string = Concat(string, S(" [oid:"), arena);
                string = Concat(string, NumberToString(identifier->attribute->id, arena), arena);
                string = Concat(string, S("]"), arena);
            }

            return string;
        }
        case EXPR_STRING: {
            TermExpression *term = (TermExpression *)expression;
            String s = term->value.string;
           
            string = Concat(string, S("'"), arena);
            string = Concat(string, s, arena);
            string = Concat(string, S("'"), arena);

            return string;
        }
        case EXPR_NUMBER: {
            TermExpression *term = (TermExpression *)expression;
            String n = NumberToString(term->value.number, arena);
            return n;
        }
        case EXPR_EQU: {
            InfixExpression *infix = (InfixExpression *)expression;

            string = Concat(string, StringifyExpression(S(""), infix->left, arena), arena);
            string = Concat(string, S(" = "), arena);
            string = Concat(string, StringifyExpression(S(""), infix->right, arena), arena);

            return string;
        }
        case EXPR_AND: {
            InfixExpression *infix = (InfixExpression *)expression;

            string = Concat(string, StringifyExpression(S(""), infix->left, arena), arena);
            string = Concat(string, S(" AND "), arena);
            string = Concat(string, StringifyExpression(S(""), infix->right, arena), arena);

            return string;
        }
        case EXPR_OR: {
            InfixExpression *infix = (InfixExpression *)expression;

            string = Concat(string, StringifyExpression(S(""), infix->left, arena), arena);
            string = Concat(string, S(" OR "), arena);
            string = Concat(string, StringifyExpression(S(""), infix->right, arena), arena);

            return string;
        }
        case EXPR_GROUP: {
            ExpressionGroup *group = (ExpressionGroup *)expression;

            string = Concat(string, S("( "), arena);
            string = Concat(string, StringifyExpression(S(""), group->expression, arena), arena);
            string = Concat(string, S(" )"), arena);

            return string;
        }
        case EXPR_ADD:
        case EXPR_SUB:
        case EXPR_MUL:
        case EXPR_DIV:
            return S("(math here)");
        case EXPR_IN_QUERY:
            return S("(Query here)");
    }

    //  Bug
    abort();
}