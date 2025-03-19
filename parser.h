#ifndef __SYSTEMR_PARSER_H__
#define __SYSTEMR_PARSER_H__

#include <common.h>
#include <sql.h>

#include <stdlib.h>

#define MAX_EXPRESSIONS 100

int ParseSQL(char *sql, size_t length);

#endif