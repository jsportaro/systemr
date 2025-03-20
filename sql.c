#include <arena.h>
#include <sql.h>

SelectStatement* CreateSelectStatement(ParsingContext *parsingContext)
{
    return &parsingContext->selectStatment;
}


SelectExpression* AppendSelectExpressionList(ParsingContext *parsingContext, SelectExpression *selectExpression)
{
    parsingContext->selectStatment.selectList[parsingContext->selectStatment.selectListCount++] = selectExpression;

    return selectExpression;
}

SelectExpression *CreateSelectExpression(ParsingContext *parsingContext, const char *as, Expression *expression)
{
    SelectExpression *selectExpression = NEW(&parsingContext->parseArena, SelectExpression);

    selectExpression->as = as;
    selectExpression->expression = expression;

    return selectExpression;
}

TableReference* AppendTableReferenceList(ParsingContext *parsingContext, const char *tableName)
{
    TableReference* table = &parsingContext->selectStatment.tables[parsingContext->selectStatment.tableCount++];

    table->identifier.type = ID_TABLE,
    table->identifier.qualifier = NULL;
    table->identifier.name = tableName;

    table->identifier.next = parsingContext->unresolved;
    parsingContext->unresolved = &table->identifier;

    return table;
}

Expression* CreateStringExpression(ParsingContext *parsingContext, const char* string)
{
    TermExpression *expression = NEW(&parsingContext->parseArena, TermExpression);
    
    expression->value.string = string;
    expression->type = EXPR_STRING;

    return (Expression *)expression;
}

Expression* CreateNumberExpression(ParsingContext *parsingContext, long number)
{
    TermExpression *expression = NEW(&parsingContext->parseArena, TermExpression);
    
    expression->value.number = number;
    expression->type = EXPR_NUMBER;

    return (Expression *)expression;
}

Expression* CreateIdentifierExpression(ParsingContext *parsingContext, const char* qualifier, const char* name)
{
    TermExpression *expression = NEW(&parsingContext->parseArena, TermExpression);

    expression->value.identifier.type = ID_COLUMN;
    expression->value.identifier.qualifier = qualifier;
    expression->value.identifier.name = name;
    expression->type = EXPR_IDENIFIER;

    expression->value.identifier.next = parsingContext->unresolved;
    parsingContext->unresolved = &expression->value.identifier;

    return (Expression *)expression;
}

Expression* CreateInfixExpression(ParsingContext *parsingContext, ExpressionType expressionType, Expression *left, Expression *right)
{
    InfixExpression *expression = NEW(&parsingContext->parseArena, InfixExpression);
    
    expression->left = left;
    expression->right = right;
    expression->type = expressionType;

    return (Expression *)expression;
}

Expression* AppendWhereExpression(ParsingContext *parsingContext, Expression *where)
{
    parsingContext->selectStatment.whereExpression = where;

    return where;
}
