#ifndef __CUSTOM_COUNTER_H
#define __CUSTOM_COUNTER_H

#include <stdlib.h>
#include <stdint.h>

typedef struct range_tracker_data_t {
	int32_t min;
	int32_t max;
} range_tracker_data_t;

typedef int32_t  range_tracker_item_t;
typedef uint32_t range_tracker_value_t;

void* range_tracker_create();
void range_tracker_destroy(void* internal);
void range_tracker_reset(range_tracker_data_t* data);
void range_tracker_get_value(range_tracker_data_t* data, range_tracker_value_t* value);
void range_tracker_push_one(range_tracker_data_t* data, range_tracker_item_t* item);
void range_tracker_push_multi(range_tracker_data_t* data, range_tracker_item_t* items, size_t n);

#endif
