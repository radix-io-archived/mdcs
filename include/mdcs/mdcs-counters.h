/*
 * Copyright (c) 2017 UChicago Argonne, LLC
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __MDCS_COUNTER_H
#define __MDCS_COUNTER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

extern mdcs_counter_type_t MDCS_COUNTER_LAST_DOUBLE;
extern mdcs_counter_type_t MDCS_COUNTER_LAST_INT64;
extern mdcs_counter_type_t MDCS_COUNTER_STAT_DOUBLE;
extern mdcs_counter_type_t MDCS_COUNTER_STAT_INT64;

typedef double mdcs_counter_last_double_item_t;
typedef double mdcs_counter_last_double_value_t;

typedef int64_t mdcs_counter_last_int64_item_t;
typedef int64_t mdcs_counter_last_int64_value_t;

typedef double mdcs_counter_stat_double_item_t;

typedef struct {
	size_t count;
	double min;
	double max;
	double avg;
	double var;
	double last;
} mdcs_counter_stat_double_value_t;

typedef int64_t mdcs_counter_stat_int64_item_t;

typedef struct {
	size_t  count;
	int64_t min;
	int64_t max;
	double  avg;
	double  var;
	int64_t last;
} mdcs_counter_stat_int64_value_t;

#ifdef __cplusplus
}
#endif

#endif
