/*
 * Copyright (c) 2017 UChicago Argonne, LLC
 *
 * See COPYRIGHT in top-level directory.
 */
#include <string.h>
#include <mdcs/mdcs.h>
#include "mdcs-hash-string.h"
#include "mdcs-counter-type.h"
#include "mdcs-global-data.h"
#include "mdcs-rpc.h"
#include "mdcs-rpc-types.h"
#include "mdcs-error.h"
#include "mdcs-counter.h"

#define MDCS_PROVIDER_ID 0

static void dummy_printer(const char* s) {}

mdcs_printer_f mdcs_print_error   = dummy_printer; // error printer
mdcs_printer_f mdcs_print_warning = dummy_printer; // warning printer

mdcs_t g_mdcs = MDCS_NULL;

int mdcs_init(margo_instance_id mid, int listening, ABT_pool pool)
{
	mdcs_t newmdcs = (mdcs_t)malloc(sizeof(struct mdcs_data_s));
	if(newmdcs == NULL) {
		MDCS_PRINT_ERROR("Could not allocate global MDCS data");
		return MDCS_ERROR;
	}

	newmdcs->counter_hash = NULL;
	newmdcs->mid = mid;

	g_mdcs = newmdcs;

	if(pool == ABT_POOL_NULL) {
		margo_get_handler_pool(mid, &pool);
	}

	g_mdcs->rpc_fetch_id = MARGO_REGISTER_PROVIDER(mid, "mdcs_fetch_counter", 
						fetch_counter_in_t, 
						fetch_counter_out_t, 
						mdcs_rpc_get_counter,
						MDCS_PROVIDER_ID, pool);

	g_mdcs->rpc_reset_id = MARGO_REGISTER_PROVIDER(mid, "mdcs_reset_counter",
						reset_counter_in_t,
						reset_counter_out_t,
						mdcs_rpc_reset_counter,
						MDCS_PROVIDER_ID, pool);

	return MDCS_SUCCESS;
}

int mdcs_initialized(int* flag)
{
	*flag = (g_mdcs != MDCS_NULL);
	return MDCS_SUCCESS;
}

int mdcs_finalize()
{
	if(g_mdcs == NULL) {
		MDCS_PRINT_ERROR("MDCS was not initialized");
		return MDCS_ERROR;
	}

	mdcs_counter_t current_counter, tmp;

	HASH_ITER(hh, g_mdcs->counter_hash, current_counter, tmp) {
		HASH_DEL(g_mdcs->counter_hash, current_counter); 
		free(current_counter->name);
		current_counter->t->destroy_f(current_counter->counter_internal_data);
		mdcs_counter_type_destroy(current_counter->t);
		if(current_counter->buffer != NULL) free(current_counter->buffer);
		free(current_counter);
	}

	free(g_mdcs);
	g_mdcs = MDCS_NULL;

	return MDCS_SUCCESS;
}

int mdcs_counter_type_create(size_t itemsize, size_t valuesize,
		mdcs_create_f create_fn, mdcs_destroy_f destroy_fn,
        mdcs_reset_f reset_fn, mdcs_push_one_f push_one_fn, 
        mdcs_push_multi_f push_multi_fn, mdcs_get_value_f get_value_fn,
        mdcs_counter_type_t* type) 
{
	if(g_mdcs == NULL) {
		MDCS_PRINT_ERROR("MDCS was not initialized");
		return MDCS_ERROR;
	}

	mdcs_counter_type_t newtype = (mdcs_counter_type_t)malloc(sizeof(struct mdcs_counter_type_s));
	if(!newtype) {
		MDCS_PRINT_ERROR("Could not allocate memory for counter type");
		return MDCS_ERROR;
	}

	newtype->counter_item_size  = itemsize;
	newtype->counter_value_size = valuesize;
	newtype->create_f           = create_fn;
	newtype->destroy_f          = destroy_fn;
	newtype->reset_f            = reset_fn;
	newtype->get_value_f        = get_value_fn;
	newtype->push_one_f         = push_one_fn;
	newtype->push_multi_f       = push_multi_fn;
	newtype->refcount           = 1;

	*type = newtype;
	return MDCS_SUCCESS; 
}

int mdcs_counter_type_destroy(mdcs_counter_type_t type) 
{
	if(g_mdcs == NULL) {
		MDCS_PRINT_ERROR("MDCS was not initialized");
		return MDCS_ERROR;
	}

	if(type == MDCS_COUNTER_TYPE_NULL) return MDCS_SUCCESS;
	
	type->refcount -= 1;
	if(type->refcount == 0) {
		free(type);
	}
	return MDCS_SUCCESS;
}

int mdcs_counter_register(const char* name,
        mdcs_counter_type_t type, size_t buffer_size, 
        mdcs_counter_t* counter) 
{
	if(g_mdcs == NULL) {
		MDCS_PRINT_ERROR("MDCS was not initialized");
		return MDCS_ERROR;
	}

	mdcs_counter_t c;
	int ret;

	uint64_t id = mdcs_hash_string(name);

	ret = mdcs_counter_find_by_id(id, &c);
	if(ret == MDCS_SUCCESS) {
		MDCS_PRINT_ERROR("Hash collision or a counter with the same name already exists");
		return MDCS_ERROR;
	}

	mdcs_counter_t newcounter = (mdcs_counter_t)malloc(sizeof(struct mdcs_counter_s));
	if(!newcounter) {
		MDCS_PRINT_ERROR("Could not allocate memory for new counter");	
		return MDCS_ERROR;
	}

	newcounter->name = strdup(name);
	newcounter->id = id;
	newcounter->t = type;
	newcounter->counter_internal_data = type->create_f();
	newcounter->buffer = NULL;
	newcounter->num_buffered = 0;
	newcounter->max_buffer_size = 0;

	if(buffer_size != 0) {
		newcounter->max_buffer_size = buffer_size;
		newcounter->buffer = malloc(buffer_size*(type->counter_item_size));
		if(newcounter->buffer == NULL) {
			MDCS_PRINT_ERROR("Could not allocate memory for counter's buffer");
			free(newcounter);
			return MDCS_ERROR;
		}
	}

	HASH_ADD(hh, g_mdcs->counter_hash, id, sizeof(uint64_t), newcounter);

	ret = mdcs_counter_reset(newcounter);
	if(ret != MDCS_SUCCESS) {
		MDCS_PRINT_WARNING("Could not reset counter");
		free(newcounter->buffer);
		free(newcounter);
		return MDCS_ERROR;
	}

	*counter = newcounter;

	return MDCS_SUCCESS;
}

int mdcs_counter_find_by_id(uint64_t id, mdcs_counter_t* counter)
{
	if(g_mdcs == NULL) {
		MDCS_PRINT_ERROR("MDCS was not initialized");
		return MDCS_ERROR;
	}

	mdcs_counter_t c;
	HASH_FIND(hh, g_mdcs->counter_hash, &id, sizeof(uint64_t), c);
    if(c == NULL) return MDCS_ERROR;
	*counter = c;
	return MDCS_SUCCESS;
}

int mdcs_counter_find_by_name(const char* name, mdcs_counter_t* counter)
{
	uint64_t id = mdcs_hash_string(name);
	return mdcs_counter_find_by_id(id, counter);
}

int mdcs_counter_push(mdcs_counter_t counter, const void* value)
{
	if(g_mdcs == NULL) {
        MDCS_PRINT_ERROR("MDCS was not initialized");
        return MDCS_ERROR;
    }

	int ret;

	if(counter == MDCS_COUNTER_NULL) {
		MDCS_PRINT_ERROR("Trying to push in a NULL counter");
		return MDCS_ERROR;
	}

	if((counter->num_buffered == counter->max_buffer_size)
	&& (counter->max_buffer_size != 0)) {
		ret = mdcs_counter_digest(counter);
		if(ret != MDCS_SUCCESS) {
			MDCS_PRINT_ERROR("Unable to digest buffer");
			return MDCS_ERROR;
		}
	}

	if(counter->max_buffer_size != 0) {
		char* p = (char*)(counter->buffer) + (counter->num_buffered)*(counter->t->counter_item_size);
		memcpy(p, value, counter->t->counter_item_size);
		counter->num_buffered += 1;
	} else {
		counter->t->push_one_f(counter->counter_internal_data, value);
	}

	return MDCS_SUCCESS;
}

int mdcs_counter_digest(mdcs_counter_t counter)
{
	if(g_mdcs == NULL) {
		MDCS_PRINT_ERROR("MDCS was not initialized");
		return MDCS_ERROR;
	}

	if(counter == MDCS_COUNTER_NULL) {
		MDCS_PRINT_ERROR("Trying to call digest on a NULL counter");
		return MDCS_ERROR;
	}

	if(counter->num_buffered != 0) {
		if(counter->t->push_multi_f != NULL) {
			counter->t->push_multi_f(counter->counter_internal_data, counter->buffer, counter->num_buffered);
		} else {
			unsigned i;
			char* value = counter->buffer;
			for(i=0; i < counter->num_buffered; i++) {
				counter->t->push_one_f(counter->counter_internal_data, value);
				value += counter->t->counter_item_size;
			}
		}
		counter->num_buffered = 0;
	}

	return MDCS_SUCCESS;
}

int mdcs_counter_value(mdcs_counter_t counter, void* value)
{
	if(g_mdcs == NULL) {
		MDCS_PRINT_ERROR("MDCS was not initialized");
		return MDCS_ERROR;
	}

	if(counter == MDCS_COUNTER_NULL) {
		MDCS_PRINT_ERROR("Trying to get value of a NULL counter");
		return MDCS_ERROR;
	}
	
	if(counter->num_buffered != 0) {
		int ret = mdcs_counter_digest(counter);
		if(ret != MDCS_SUCCESS) {
			MDCS_PRINT_ERROR("Could not digest counter's buffer");
			return MDCS_ERROR;
		}
	}
	counter->t->get_value_f(counter->counter_internal_data, value);
	
	return MDCS_SUCCESS;
}

int mdcs_counter_reset(mdcs_counter_t counter)
{
	if(g_mdcs == NULL) {
		MDCS_PRINT_ERROR("MDCS was not initialized");
		return MDCS_ERROR;
	}

	if(counter == MDCS_COUNTER_NULL) {
		MDCS_PRINT_ERROR("Trying to reset a NULL counter");
		return MDCS_ERROR;
	}

	counter->t->reset_f(counter->counter_internal_data);

	return MDCS_SUCCESS;
}

int mdcs_set_error_printer(mdcs_printer_f fun)
{
	mdcs_print_error = fun;
	return MDCS_SUCCESS;
}

int mdcs_set_warning_printer(mdcs_printer_f fun)
{
	mdcs_print_warning = fun;
	return MDCS_SUCCESS;
}
