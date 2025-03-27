#include <arena.h>
#include <sql.h>

#include <string.h>

static bool AddTableLookup(ParsingContext *parsingContext, TableReference *tableReference, const char *name)
{
    size_t nameLength = strlen(name);
    uint32_t i = Hash(name, nameLength) % MAX_HASH_SIZE;

    //  If neither name exists, we're good to add.
    //  However, if a reference already exists in either case then we have a duplicate name collision
    for (;;)
    {
        if (parsingContext->tableLookup[i] == NULL)
        {
            parsingContext->tableLookup[i] = tableReference;

            return true;
        }
        else if ((strncmp(name, parsingContext->tableLookup[i]->name, nameLength) == 0) ||
                 (strncmp(name, parsingContext->tableLookup[i]->alias, nameLength) == 0) )
        {
            //  Name already exists as either a table name or alias.  Fail
            return false;
        }

        i = (i + 1) % MAX_HASH_SIZE;
    }
}

static bool VerifyAliasedColumns(ParsingContext *parsingContext, SelectExpressionList *selectExpressionList)
{
    //  Look through Identifiers to see if they match up
    //  with table names.
    //  For identifiers with a qualifier
    //     -> If matches a table; skip
    //     -> If matches a alias; replace qualifier with table name
    //     -> If matches neither table or alias; error!
    for (int i = 0; i < selectExpressionList->selectListCount; i++)
    {
        SelectExpression *expression = selectExpressionList->selectList[i];
        Identifier *unresolved = expression->unresolved;
        while (unresolved != NULL)
        {
            if (unresolved->qualifier != NULL)
            {
                size_t length = strlen(unresolved->qualifier);
                uint32_t i = Hash(unresolved->qualifier, length) % MAX_HASH_SIZE;

                for (;;)
                {
                    if (parsingContext->tableLookup[i] == NULL)
                    {
                        return false;
                    }
                    else if (strncmp(unresolved->qualifier, parsingContext->tableLookup[i]->alias, length) == 0)
                    {
                        //  Name already exists as either a table name or alias.  Fail
                        break;
                    }

                    i = (i + 1) % MAX_HASH_SIZE;
                }
            }

            unresolved = unresolved->next;
        }
    }

    return true;
}

void Finalize(ParsingContext *parsingContext, SelectStatement* selectStatement)
{
    parsingContext->selectStatement = selectStatement;
}



SelectStatement *CreateSelectStatement(ParsingContext *parsingContext, SelectExpressionList *selectExpressionList, TableReferenceList *tableReferenceList, Expression *whereExpression)
{
    SelectStatement *selectStatement = NEW(&parsingContext->parseArena, SelectStatement);

    selectStatement->selectExpressionList = selectExpressionList;
    selectStatement->tableReferenceList = tableReferenceList;
    selectStatement->whereExpression = whereExpression;

    parsingContext->success &= VerifyAliasedColumns(parsingContext, selectExpressionList);
    memset(parsingContext->tableLookup, 0, MAX_HASH_SIZE); 

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
    selectExpression->unresolved = parsingContext->unresolved;

    parsingContext->unresolved = NULL;

    return selectExpression;
}

TableReferenceList *CreateTableReferenceList(ParsingContext *parsingContext, TableReference *tableReference)
{
    TableReferenceList *tableReferenceList = NEW(&parsingContext->parseArena, TableReferenceList);

    return AppendTableReferenceList(parsingContext, tableReferenceList, tableReference);;
}

TableReferenceList *AppendTableReferenceList(ParsingContext *parsingContext, TableReferenceList *tableReferenceList, TableReference *tableReference)
{

    tableReferenceList->tableReferences [tableReferenceList->count++] = tableReference;

    return tableReferenceList;
}

TableReference *CreateTableReference(ParsingContext *parsingContext, const char *tableName, const char *tableAlias)
{
    TableReference *tableReference = NEW(&parsingContext->parseArena, TableReference);

    tableReference->name = tableName;
    tableReference->alias = tableAlias == NULL ? tableName : tableAlias;

    bool wasAdded = AddTableLookup(parsingContext, tableReference, tableReference->alias);

    return tableReference;
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
