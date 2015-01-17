#pragma once

//===	Includes

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <pcre.h>

//===	Defines

#define	REGEX_OVECSIZE 1023

//===	Variables

extern int	regex_init;
extern pcre	*regex_query_full, *regex_query_structure;
extern pcre_extra	*regex_study_query_full, *regex_study_query_structure;

//===	Functions

int RegexInitialize	(void);
void RegexCleanup	(void);
