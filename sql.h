#ifndef __SYSTEMR_SQL_H__
#define __SYSTEMR_SQL_H__

#define UNUSED(x) (void)(x)

typedef struct
{
    const char *qualifier;
    const char *name;
} Identifier;

typedef enum
{
    EXPR_INTEGER,
    EXPR_STRING,
    EXPR_IDENIFIER,
    EXPR_ADD,
    EXPR_SUB,
    EXPR_MUL,
    EXPR_DIV,
    EXPR_EQU,
    EXPR_AND,
    EXPR_OR,
} ExpressionType;

typedef struct
{
    ExpressionType type;
} Expression;

typedef struct infix_expr
{
    ExpressionType type;

    Expression *left;
    Expression *right;
} InfixExpression;

typedef struct term_expr
{
    ExpressionType type;

    union 
    {
        int number;
        char *string;
        Identifier *identifier;
    } value;
} TermExpression;

typedef struct {
    int i;
} SelectStatement;

typedef struct {
    int i;
} SelectExpression;

typedef struct {
    int i;
} TableReference;

typedef struct {
    int i;
} WhereExpression;

SelectStatement* CreateSelectStatement(SelectExpression *selectExpression, TableReference *tableReferences, WhereExpression *whereExpressions);
SelectExpression* CreateSelectExpressionList(Expression *expression);
SelectExpression* AppendSelectExpressionList(SelectExpression* selectExpressionList, Expression *expression);
TableReference* CreateTableReferenceList(TableReference* tableReference);
TableReference* AppendTableReferenceList(TableReference* tableReferenceList, TableReference* tableReference);
TableReference* CreateTableReference(const char *name);

Expression* CreateStringExpression(const char* string);
Expression* CreateNumberExpression(long number);
Expression* CreateIdentifierExpression(const char* qualifier, const char* name);
Expression* CreateInfixExpression(ExpressionType expressionType, Expression *left, Expression *right);

WhereExpression* CreateWhereExpression(Expression *where);

#endif