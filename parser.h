#ifndef __SYSTEMR_PARSER_H__
#define __SYSTEMR_PARSER_H__

#include <common.h>
#include <sql.h>

#include <stdlib.h>

ParsingContext ParseSQL(const char *sql, size_t length, Arena *arena);

#endif
