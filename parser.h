#ifndef __SYSTEMR_PARSER_H__
#define __SYSTEMR_PARSER_H__

#include <common.h>
#include <sql.h>

#include <stdlib.h>

void ParseSQL(ParsingContext *parsingContext, const char *sql, size_t length);

#endif
