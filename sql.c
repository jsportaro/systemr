#include <sql.h>

SelectStatement* CreateSelectStatement(ParsingContext *parsingContext)
{
    UNUSED(parsingContext);
    return &parsingContext->selectStatment;
}


Expression* AppendSelectExpressionList(ParsingContext *parsingContext, Expression *expression)
{
    UNUSED(parsingContext);
    UNUSED(expression);
    return &parsingContext->expressions[0];
}

TableReference* AppendTableReferenceList(ParsingContext *parsingContext, const char *tableName)
{
    TableReference* table = &parsingContext->selectStatment.tables[parsingContext->selectStatment.tableCount++];

    table->name = tableName;

    return table;
}

Expression* CreateStringExpression(ParsingContext *parsingContext, const char* string)
{
    UNUSED(string);
    return &parsingContext->expressions[0];
}

Expression* CreateNumberExpression(ParsingContext *parsingContext, long number)
{
    UNUSED(number);
    return &parsingContext->expressions[0];
}

Expression* CreateIdentifierExpression(ParsingContext *parsingContext, const char* qualifier, const char* name)
{
    UNUSED(qualifier);
    UNUSED(name);
    return &parsingContext->expressions[0];
}

Expression* CreateInfixExpression(ParsingContext *parsingContext, ExpressionType expressionType, Expression *left, Expression *right)
{
    UNUSED(expressionType);
    UNUSED(left);
    UNUSED(right);
    return &parsingContext->expressions[0];
}

Expression* AppendWhereExpression(ParsingContext *parsingContext, Expression *where)
{
    parsingContext->selectStatment.whereExpression = where;

    return where;
}