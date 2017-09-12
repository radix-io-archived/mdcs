/*
 * Copyright (c) 2017 UChicago Argonne, LLC
 *
 * See COPYRIGHT in top-level directory.
 */
#include "mdcs-hash-string.h"

uint64_t mdcs_hash_string(const char *string)
{
	uint64_t result = 5381;
	const unsigned char *p;

	p = (const unsigned char *) string;

	while (*p != '\0') {
		result = (result << 5) + result + *p;
		++p;
	}
	return result;
}
