#include <string.h>
#include <mdcs/mdcs.h>
#include "hash_string.h"
#include "mdcs-counter-type.h"
#include "mdcs-data.h"
#include "rpc.h"
#include "rpc-types.h"
#include "uthash.h"

struct mdcs_counter_s {
	char* name;             // name of the counter
	uint64_t id;            // id of the counter
	mdcs_counter_type_t t;  // counter type (including accessor functions)
	void* counter_data;     // data attached to the counter
	void* buffer;			// buffer to hold pushed values
	size_t max_buffer_size; // maximum number of elements the buffer can hold
	size_t num_buffered;    // number of elements currently in the buffer
	UT_hash_handle hh;      // counters are placed in a hash by id
};

static void dummy_printer(const char* s) {}

mdcs_printer_f mdcs_print_error   = dummy_printer; // error printer
mdcs_printer_f mdcs_print_warning = dummy_printer; // warning printer

mdcs_t g_mdcs = MDCS_NULL;

int mdcs_init(margo_instance_id mid, int listening)
{
	mdcs_t newmdcs = (mdcs_t)malloc(sizeof(struct mdcs_data_s));
	if(newmdcs == NULL) return MDCS_ERROR;

	newmdcs->counter_hash = NULL;
	newmdcs->mid = mid;

	g_mdcs = newmdcs;

	g_mdcs->rpc_fetch_id = MARGO_REGISTER(mid, "mdcs_fetch_counter", 
							get_counter_in_t, get_counter_out_t, mdcs_rpc_get_counter);
	g_mdcs->rpc_reset_id = MARGO_REGISTER(mid, "mdcs_reset_counter",
							reset_counter_in_t, reset_counter_out_t, mdcs_rpc_reset_counter);

	return MDCS_SUCCESS;
}

int mdcs_initialized(int* flag)
{
	*flag = (g_mdcs != MDCS_NULL);
	return MDCS_SUCCESS;
}

int mdcs_finalize()
{
	mdcs_counter_t current_counter, tmp;

	HASH_ITER(hh, g_mdcs->counter_hash, current_counter, tmp) {
		HASH_DEL(g_mdcs->counter_hash, current_counter); 
		free(current_counter->name);
		mdcs_counter_type_destroy(current_counter->t);
		free(current_counter->counter_data);
		if(current_counter->buffer != NULL) free(current_counter->buffer);
		free(current_counter);
	}

	free(g_mdcs);
	g_mdcs = MDCS_NULL;

	return MDCS_SUCCESS;
}

int mdcs_counter_type_create(size_t counterdatasize, size_t valuesize, 
        mdcs_reset_f reset_fn, mdcs_push_one_f push_one_fn, 
        mdcs_push_multi_f push_multi_fn, mdcs_get_value_f get_value_fn,
        mdcs_counter_type_t* type) 
{
	mdcs_counter_type_t newtype = (mdcs_counter_type_t)malloc(sizeof(struct mdcs_counter_type_s));
	if(!newtype) return MDCS_ERROR;

	newtype->counter_value_size = valuesize;
    newtype->counter_data_size  = counterdatasize; 
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
	if(type == MDCS_COUNTER_TYPE_NULL) return MDCS_ERROR;
	
	type->refcount -= 1;
	if(type->refcount == 0) {
		free(type);
	}
	return MDCS_SUCCESS;
}

int mdcs_counter_register(const char* name,
        mdcs_counter_type_t type, size_t buffer_size, 
        mdcs_counter_t* counter) {
	
	mdcs_counter_t c;
	int ret;

	uint64_t id = mdcs_hash_string(name);

	ret = mdcs_counter_find_by_id(id, &c);
	if(ret == MDCS_SUCCESS) return MDCS_ERROR;

	mdcs_counter_t newcounter = (mdcs_counter_t)malloc(sizeof(struct mdcs_counter_s));
	if(!newcounter) return MDCS_ERROR;
	newcounter->name = strdup(name);
	newcounter->id = id;
	newcounter->t = type;
	newcounter->counter_data = malloc(type->counter_data_size);
	newcounter->buffer = NULL;
	newcounter->num_buffered = 0;
	newcounter->max_buffer_size = 0;
	if(buffer_size != 0) {
		newcounter->max_buffer_size = buffer_size;
		newcounter->buffer = malloc(buffer_size*(type->counter_value_size));
	}

	HASH_ADD(hh, g_mdcs->counter_hash, id, sizeof(uint64_t), newcounter);

	ret = mdcs_counter_find_by_id(id, &c);
	if(ret != MDCS_SUCCESS) return MDCS_ERROR;

	*counter = newcounter;
	mdcs_counter_reset(newcounter);

	return MDCS_SUCCESS;
}

int mdcs_counter_find_by_id(uint64_t id, mdcs_counter_t* counter)
{
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
	if(counter == MDCS_COUNTER_NULL) return MDCS_ERROR;

	if((counter->num_buffered == counter->max_buffer_size)
	&& (counter->max_buffer_size != 0)) {
		mdcs_counter_digest(counter);
	}

	if(counter->max_buffer_size != 0) {
		char* p = (char*)(counter->buffer) + (counter->num_buffered)*(counter->t->counter_value_size);
		memcpy(p, value, counter->t->counter_value_size);
		counter->num_buffered += 1;
	} else {
		counter->t->push_one_f(counter->counter_data, value);
	}

	return MDCS_SUCCESS;
}

int mdcs_counter_digest(mdcs_counter_t counter)
{
	if(counter == MDCS_COUNTER_NULL) return MDCS_ERROR;

	if(counter->num_buffered != 0) {
		if(counter->t->push_multi_f != NULL) {
			counter->t->push_multi_f(counter->counter_data, counter->buffer, counter->num_buffered);
		} else {
			unsigned i;
			char* value = counter->buffer;
			for(i=0; i < counter->num_buffered; i++) {
				counter->t->push_one_f(counter->counter_data, value);
				value += counter->t->counter_value_size;
			}
		}
	}

	return MDCS_SUCCESS;
}

int mdcs_counter_value(mdcs_counter_t counter, void* value)
{
	if(counter == MDCS_COUNTER_NULL) return MDCS_ERROR;
	
	if(counter->num_buffered != 0)
		mdcs_counter_digest(counter);
	counter->t->get_value_f(counter->counter_data, value);
	
	return MDCS_SUCCESS;
}

int mdcs_counter_reset(mdcs_counter_t counter)
{
	if(counter == MDCS_COUNTER_NULL) return MDCS_ERROR;

	counter->t->reset_f(counter->counter_data);

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
