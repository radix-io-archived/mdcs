/*
 * Copyright (c) 2017 UChicago Argonne, LLC
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef __MDCS_COUNTER_TYPE_H
#define __MDCS_COUNTER_TYPE_H

#include <stdint.h>

struct mdcs_counter_type_s {
	size_t            counter_item_size;  // size of items pushed into the counter
	size_t            counter_value_size; // size of the values read from the counter 
	size_t            counter_data_size;  // size of the internal data
	mdcs_reset_f      reset_f;            // function used to reset the counter to a given value
	mdcs_get_value_f  get_value_f;        // function used to get the value of the counter
	mdcs_push_one_f   push_one_f;         // function used to push a new value to a counter
	mdcs_push_multi_f push_multi_f;       // function used to push multiple values to a counter
	int refcount;                         // number of objects pointing to this counter type
};

#endif
