#ifndef __MOCHI_MDCS_SERVICE_H
#define __MOCHI_MDCS_SERVICE_H

#include <mdcs/mdcs.h>

typedef void  (*mdcs_reset_f)(void* counter_data);
typedef void  (*mdcs_get_value_f)(void* counter_data, void* val);
typedef void  (*mdcs_push_one_f)(void* counter_data, const void* val);
typedef void  (*mdcs_push_multi_f)(void* counter_data, const void* val, size_t num);

/* The following structures are placed here for reference 
   but their implementation is located in mdcs-service.c

struct mdcs_counter_type_s {
	size_t            counter_size;      // size of the value of the counter (not counter data)
	size_t            counter_data_size; // size of the internal data
	mdcs_reset_f      reset_fn;          // function used to reset the counter to a given value
	mdcs_get_value_f  get_value_f;       // function used to get the value of the counter
	mdcs_push_one_f   push_one_f;        // function used to push a new value to a counter
	mdcs_push_multi_f push_multi_f;      // function used to push multiple values to a counter
	int refcount;                        // number of objects pointing to this counter type
};

struct mdcs_namespace_s {
	char* name;                          // name of the namespace
	mdcs_counter_t counter_hash_by_name; // root of counter hash, by name
    ABT_rwlock namespace_rwlock;         // rwlock protecting access to counters
	UT_hash_handle hh;                   // UThash for the hash of namespaces by name
};

struct mdcs_counter_s {
	char* name;             // name of the counter
	uint64_t id;            // id of the counter
	mdcs_counter_type_t fn; // counter type (including accessor functions)
	void* counter_data;     // data attached to the counter
	mdcs_namespace_t owner; // enclosing namespace
	UT_hash_handle name_hh; // counters are placed in a hash by name within a namespace
	UT_hash_handle id_hh;   // counters are placed in a hash by id within the mdcs data
	
};

struct mdcs_data_s {
	mdcs_namespace_t 	namespace_hash_by_name; // hash of namespaces by name
	mdcs_namespace_t    namespace_hash_by_id;   // hash of namespaces by id
	mdcs_counter_t      counter_hash_by_id;     // hash of counters by id
	size_t              total_num_counters;     // total number of counters
	ABT_rwlock 			namespace_rwlock;       // rwlock protecting access to data
};

*/

typedef struct mdcs_namespace_s* 	mdcs_namespace_t;
typedef struct mdcs_counter_type_s* mdcs_counter_type_t;
typedef struct mdcs_counter_s* 		mdcs_counter_t;

#define MDCS_NAMESPACE_NULL    ((mdcs_namespace_t)NULL)
#define MDCS_COUNTER_NULL      ((mdcs_counter_t)NULL)
#define MDCS_COUNTER_TYPE_NULL ((mdcs_counter_type_t)NULL)

/**
 * Creates a new namespace to hold counters. The call will fail if
 * a namespace already exists with the same name.
 *
 * \param[in] name Name of the namespace.
 * \param[out] namespace Newly created namespace.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_namespace_create(const char* name, mdcs_namespace_id_t* namespace);

/**
 * Destroy a namespace. This will destroy all counters associated with
 * this namespace. Any attempt to access such counters (even through RPCs)
 * after the namespace is destroyed will lead to undefined behavior.
 *
 * \param[in] namespace Namespace to destroy.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_namespace_destroy(mdcs_namespace_id_t namespace);

/**
 * Creates a new counter type. A counter type is defined by providing
 * the size of the counter's internal data (including counter size),
 * the size of the counter's value, the function used to reset the
 * counter, the function used to push a value, a function used to
 * push several values at once, and the function used o read the current value.
 *
 * \param[in] counterdatasize Size of the internal data structure of the counter.
 * \param[in] valuesize Size of the value of the counter.
 * \param[in] reset_fn Function used to reset the counter.
 * \param[in] push_one_fn Function used to push new values to the counter.
 * \param[in] push_multi_fn Function used to push multiple values to the counter.
 * \param[in] get_value_fn Function used to read the current value.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_counter_type_create(size_t counterdatasize, size_t valuesize, 
        mdcs_reset_f reset_fn, mdcs_push_one_f push_one_fn, 
        mdcs_push_multi_f push_multi_fn, mdcs_get_value_f get_value_fn,
        mdcs_counter_type_t* type);

/**
 * Destroyes a counter type. This function only destroyes the counter type
 * if no existing counter use this type. Hence it is safe to destroy all counter
 * types right after having created counters of that type.
 *
 * \param[in] type Counter type to destroy.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */		
int mdcs_counter_type_destroy(mdcs_counter_type_t type);

/**
 * Adds a new counter to a given namespace. Will fail if the name of the
 * counter already exists in that namespace.
 *
 * \param[in] namespace Namespace in which to add the counter.
 * \param[in] name Name of the counter.
 * \param[in] type Type of counter.
 * \param[in] buffer_size Size of the buffer to cache counter values (can be 0).
 * \param[out] counter Newly created counter.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_namespace_add_counter(mdcs_namespace_t namespace, const char* name,
        mdcs_counter_type_id_t type, size_t buffer_size, 
        mdcs_counter_t* counter);

/**
 * Pushes a value into a counter.
 * 
 * \param[in] counter Counter in which to push a value.
 * \param[in] value Pointer to the value to push.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_counter_push(mdcs_counter_t counter, const void* value);

/**
 * Forces all values present in the buffer to be pushed into the counter,
 * and empties the buffer.
 * 
 * \param[in] counter Counter whose buffer should be digested.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_counter_digest(mdcs_counter_t counter);

/**
 * Get the current value of the counter.
 * 
 * \param[in] counter Counter from which to retrieve the value.
 * \param[out] value Pointer to the location where the value should be placed.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_counter_value(mdcs_counter_t counter, void* value);

/**
 * Reset the value of the counter. This will also erase the content of the
 * buffer, if any.
 * 
 * \param[in] counter whose value should be reset.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_counter_reset(mdcs_counter_t counter);

#endif