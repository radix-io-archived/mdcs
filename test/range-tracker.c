#include <limits.h>
#include "range-tracker.h"

void* range_tracker_create()
{
	return malloc(sizeof(range_tracker_data_t));
}

void range_tracker_destroy(void* internal)
{
	free(internal);
}

void range_tracker_reset(range_tracker_data_t* data) 
{
	data->min = INT_MAX;
	data->max = INT_MIN;
}

void range_tracker_get_value(range_tracker_data_t* data, range_tracker_value_t* value)
{
	if(data->min == INT_MAX && data->max == INT_MIN) {
		*value = 0;
	} else {
		*value = (range_tracker_value_t)(data->max - data->min);
	}
}

void range_tracker_push_one(range_tracker_data_t* data, range_tracker_item_t* item)
{
	if(data->min > *item) {
		data->min = *item;
	}
	if(data->max < *item) {
		data->max = *item;
	}
}

void range_tracker_push_multi(range_tracker_data_t* data, range_tracker_item_t* items, size_t n)
{
	size_t i;
	for(i=0; i<n; i++) {
		range_tracker_item_t* x = items + i;
		if(data->min > *x) data->min = *x;
		if(data->max < *x) data->max = *x;
	}
}

