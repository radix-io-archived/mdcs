/*
 * Copyright (c) 2017 UChicago Argonne, LLC
 *
 * See COPYRIGHT in top-level directory.
 */
#ifndef MDCS_COUNTER_H
#define MDCS_COUNTER_H

#include "uthash.h"

struct mdcs_counter_s {
	char* name;                  // name of the counter
	uint64_t id;                 // id of the counter
	mdcs_counter_type_t t;       // counter type (including accessor functions)
	void* counter_internal_data; // data attached to the counter
	void* buffer;                // buffer to hold pushed values
	size_t max_buffer_size;      // maximum number of elements the buffer can hold
	size_t num_buffered;         // number of elements currently in the buffer
	UT_hash_handle hh;           // counters are placed in a hash by id
};

#endif
