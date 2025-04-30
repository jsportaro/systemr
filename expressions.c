#include <arena.h>
#include <common.h>
#include <expressions.h>
#include <rstrings.h>

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
    }
}