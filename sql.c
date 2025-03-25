#include <arena.h>
#include <sql.h>

void Finalize(ParsingContext *parsingContext, SelectStatement* selectStatement)
{
    parsingContext->selectStatement = selectStatement;
}

SelectStatement *CreateSelectStatement(ParsingContext *parsingContext, SelectExpressionList *selectExpressionList, TableReferenceList *tableReferenceList, Expression *whereExpression)
{
    UNUSED(parsingContext);

    SelectStatement *selectStatement = NEW(&parsingContext->parseArena, SelectStatement);

    selectStatement->selectExpressionList = selectExpressionList;
    selectStatement->tableReferenceList = tableReferenceList;
    selectStatement->whereExpression = whereExpression;

    return selectStatement;
}

SelectExpressionList *CreateSelectExpressionList(ParsingContext *parsingContext, SelectExpression *selectExpression)
{
    SelectExpressionList *selectExpressionList = NEW(&parsingContext->parseArena, SelectExpressionList);

    return AppendSelectExpressionList(parsingContext, selectExpressionList, selectExpression);;
}

SelectExpressionList *AppendSelectExpressionList(ParsingContext *parsingContext, SelectExpressionList *selectExpressionList, SelectExpression *selectExpression)
{
    UNUSED(parsingContext);

    selectExpressionList->selectList[selectExpressionList->selectListCount++] = selectExpression;

    return selectExpressionList;
}

SelectExpression *CreateSelectExpression(ParsingContext *parsingContext, const char *as, Expression *expression)
{
    SelectExpression *selectExpression = NEW(&parsingContext->parseArena, SelectExpression);

    selectExpression->as = as;
    selectExpression->expression = expression;

    return selectExpression;
}

TableReferenceList *CreateTableReferenceList(ParsingContext *parsingContext, const char *tableName)
{
    TableReferenceList *tableReferenceList = NEW(&parsingContext->parseArena, TableReferenceList);

    return AppendTableReferenceList(parsingContext, tableReferenceList, tableName);;
}

TableReferenceList *AppendTableReferenceList(ParsingContext *parsingContext, TableReferenceList *tableReferenceList, const char *tableName)
{
    TableReference *tableReference = NEW(&parsingContext->parseArena, TableReference);

    tableReference->name = tableName;
    tableReferenceList->tableReferences [tableReferenceList->count++] = tableReference;

    return tableReferenceList;
}

Expression *CreateStringExpression(ParsingContext *parsingContext, const char* string)
{
    TermExpression *expression = NEW(&parsingContext->parseArena, TermExpression);
    
    expression->value.string = string;
    expression->type = EXPR_STRING;

    return (Expression *)expression;
}

Expression *CreateNumberExpression(ParsingContext *parsingContext, long number)
{
    TermExpression *expression = NEW(&parsingContext->parseArena, TermExpression);
    
    expression->value.number = number;
    expression->type = EXPR_NUMBER;

    return (Expression *)expression;
}

Expression *CreateIdentifierExpression(ParsingContext *parsingContext, const char* qualifier, const char* name)
{
    TermExpression *expression = NEW(&parsingContext->parseArena, TermExpression);

    expression->value.identifier.qualifier = qualifier;
    expression->value.identifier.name = name;
    expression->type = EXPR_IDENIFIER;

    expression->value.identifier.next = parsingContext->unresolved;
    parsingContext->unresolved = &expression->value.identifier;

    return (Expression *)expression;
}

Expression *CreateInfixExpression(ParsingContext *parsingContext, ExpressionType expressionType, Expression *left, Expression *right)
{
    InfixExpression *expression = NEW(&parsingContext->parseArena, InfixExpression);
    
    expression->left = left;
    expression->right = right;
    expression->type = expressionType;

    return (Expression *)expression;
}

Expression *CreateInExpression(ParsingContext *parsingContext, Expression *left, SelectStatement *selectStatement)
{
    InQueryExpression *expression = NEW(&parsingContext->parseArena, InQueryExpression);

    expression->left = left;
    expression->query = selectStatement;
    expression->type = EXPR_IN_QUERY;

    return (Expression *)expression;
}

Expression *AppendWhereExpression(ParsingContext *parsingContext, Expression *where)
{
    UNUSED(parsingContext);
    
    return where;
}
