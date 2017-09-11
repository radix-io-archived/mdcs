#include <mdcs/mdcs-service.h>
#include <mdcs/mdcs-counters.h>
#include <mdcs/mdcs.h>
#include "mdcs-counter-type.h"

////////////////////////////////////////////////////////////////////////////
// Simple double value counter, tracks the last pushed value
////////////////////////////////////////////////////////////////////////////
typedef struct {
	double value;
} mdcs_counter_last_double_internal;

static void last_double_reset(
	mdcs_counter_last_double_internal* internal)
{
	internal->value = 0;
}

static void last_double_get_value(
	mdcs_counter_last_double_internal* internal,
	double* v)
{
	*v = internal->value;
}

static void last_double_push_one(
	mdcs_counter_last_double_internal* internal,
	double* v)
{
	internal->value = *v;
}

static void last_double_push_multi(
	mdcs_counter_last_double_internal* internal,
    double* v, size_t count)
{
	internal->value = v[count-1];
}

struct mdcs_counter_type_s MDCS_COUNTER_LAST_DOUBLE_S = {
   	.counter_value_size = sizeof(mdcs_counter_last_double_t), 
	.counter_data_size  = sizeof(mdcs_counter_last_double_internal),
    .reset_f            = (mdcs_reset_f)last_double_reset,
    .get_value_f        = (mdcs_get_value_f)last_double_get_value,
    .push_one_f         = (mdcs_push_one_f)last_double_push_one,
    .push_multi_f       = (mdcs_push_multi_f)last_double_push_multi,
    .refcount           = -1
};

////////////////////////////////////////////////////////////////////////////
// Simple int64 value counter, tracks the last pushed value
////////////////////////////////////////////////////////////////////////////
typedef struct {
	int64_t value;
} mdcs_counter_last_int64_internal;

static void last_int64_reset(
	mdcs_counter_last_int64_internal* internal)
{
	internal->value = 0;
}

static void last_int64_get_value(
	mdcs_counter_last_int64_internal* internal,
	int64_t* v)
{
	*v = internal->value;
}

static void last_int64_push_one(
	mdcs_counter_last_int64_internal* internal,
	int64_t* v)
{
	internal->value = *v;
}

static void last_int64_push_multi(
	mdcs_counter_last_int64_internal* internal,
	int64_t* v, size_t count)
{
	internal->value = v[count-1];
}

struct mdcs_counter_type_s MDCS_COUNTER_LAST_INT64_S = {
   	.counter_value_size = sizeof(mdcs_counter_last_int64_t), 
	.counter_data_size  = sizeof(mdcs_counter_last_int64_internal),
    .reset_f            = (mdcs_reset_f)last_int64_reset,
    .get_value_f        = (mdcs_get_value_f)last_int64_get_value,
    .push_one_f         = (mdcs_push_one_f)last_int64_push_one,
    .push_multi_f       = (mdcs_push_multi_f)last_int64_push_multi,
    .refcount           = -1
};

////////////////////////////////////////////////////////////////////////////
// Statistics counter, tracks statistics of double values
////////////////////////////////////////////////////////////////////////////
typedef mdcs_counter_stat_double_t mdcs_counter_stat_double_internal;

static void stat_double_reset(
	mdcs_counter_stat_double_internal* internal)
{
	memset(internal,0,sizeof(mdcs_counter_stat_double_internal));
}

static void stat_double_get_value(
	mdcs_counter_stat_double_internal* internal,
	mdcs_counter_stat_double_t* v)
{
	memcpy(v,internal, sizeof(mdcs_counter_stat_double_t));
}

static void stat_double_push_one(
	mdcs_counter_stat_double_internal* internal,
	double* v)
{
	double x = *v;
	internal->last = x;
	internal->count += 1;
	if(internal->count == 1) {
		internal->max = x;
		internal->min = x;
		internal->avg = x;
		internal->var = 0.0;
	} else {
		if(x > internal->max)
			internal->max = x;
		if(x < internal->min)
			internal->min = x;
		double old_avg = internal->avg;
		double k = internal->count - 1;
		double p = k/(k+1);
		internal->avg = p*old_avg + (x/k);
		double new_avg = internal->avg;
		double old_var = internal->var;
		internal->var = p*(old_var + old_avg*old_avg)
						+ x*x/(k+1) - new_avg*new_avg;
	}
}

struct mdcs_counter_type_s MDCS_COUNTER_STAT_DOUBLE_S = {
   	.counter_value_size = sizeof(mdcs_counter_stat_double_t), 
	.counter_data_size  = sizeof(mdcs_counter_stat_double_internal),
    .reset_f            = (mdcs_reset_f)stat_double_reset,
    .get_value_f        = (mdcs_get_value_f)stat_double_get_value,
    .push_one_f         = (mdcs_push_one_f)stat_double_push_one,
    .push_multi_f       = (mdcs_push_multi_f)NULL,
    .refcount           = -1
};

////////////////////////////////////////////////////////////////////////////
// Statistics counter, tracks statistics of int64 values
////////////////////////////////////////////////////////////////////////////
typedef mdcs_counter_stat_int64_t mdcs_counter_stat_int64_internal;

static void stat_int64_reset(
	mdcs_counter_stat_int64_internal* internal)
{
	memset(internal,0,sizeof(mdcs_counter_stat_int64_internal));
}

static void stat_int64_get_value(
	mdcs_counter_stat_int64_internal* internal,
	mdcs_counter_stat_int64_t* v)
{
	memcpy(v,internal, sizeof(mdcs_counter_stat_int64_t));
}

static void stat_int64_push_one(
	mdcs_counter_stat_int64_internal* internal,
	int64_t* v)
{
	int64_t x = *v;
	internal->last = x;
	internal->count += 1;
	if(internal->count == 1) {
		internal->max = x;
		internal->min = x;
		internal->avg = x;
		internal->var = 0.0;
	} else {
		if(x > internal->max)
			internal->max = x;
		if(x < internal->min)
			internal->min = x;
		double old_avg = internal->avg;
		double k = internal->count - 1;
		double p = k/(k+1);
		internal->avg = p*old_avg + (x/k);
		double new_avg = internal->avg;
		double old_var = internal->var;
		internal->var = p*(old_var + old_avg*old_avg)
						+ x*x/(k+1) - new_avg*new_avg;
	}
}

struct mdcs_counter_type_s MDCS_COUNTER_STAT_INT64_S = {
   	.counter_value_size = sizeof(mdcs_counter_stat_int64_t), 
	.counter_data_size  = sizeof(mdcs_counter_stat_int64_internal),
    .reset_f            = (mdcs_reset_f)stat_int64_reset,
    .get_value_f        = (mdcs_get_value_f)stat_int64_get_value,
    .push_one_f         = (mdcs_push_one_f)stat_int64_push_one,
    .push_multi_f       = (mdcs_push_multi_f)NULL,
    .refcount           = -1
};

////////////////////////////////////////////////////////////////////////////
// Variables exposed to users
////////////////////////////////////////////////////////////////////////////
mdcs_counter_type_t MDCS_COUNTER_LAST_DOUBLE = &MDCS_COUNTER_LAST_DOUBLE_S;
mdcs_counter_type_t MDCS_COUNTER_LAST_INT64  = &MDCS_COUNTER_LAST_INT64_S;
mdcs_counter_type_t MDCS_COUNTER_STAT_DOUBLE = &MDCS_COUNTER_STAT_DOUBLE_S;
mdcs_counter_type_t MDCS_COUNTER_STAT_INT64  = &MDCS_COUNTER_STAT_INT64_S;

