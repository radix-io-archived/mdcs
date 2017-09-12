#ifndef __MOCHI_MDCS_H
#define __MOCHI_MDCS_H

#include <stdlib.h>
#include <stdint.h>
#include <margo.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MDCS_SUCCESS 0
#define MDCS_ERROR  (-1)
#define MDCS_TRUE    1
#define MDCS_FALSE   0

typedef void  (*mdcs_reset_f)(void* counter_data);
typedef void  (*mdcs_get_value_f)(void* counter_data, void* val);
typedef void  (*mdcs_push_one_f)(void* counter_data, const void* val);
typedef void  (*mdcs_push_multi_f)(void* counter_data, const void* val, size_t num);
typedef struct mdcs_counter_type_s* mdcs_counter_type_t;
typedef struct mdcs_counter_s*      mdcs_counter_t;
typedef uint64_t                    mdcs_counter_id_t;

#define MDCS_COUNTER_NULL      ((mdcs_counter_t)NULL)
#define MDCS_COUNTER_TYPE_NULL ((mdcs_counter_type_t)NULL)

/**
 * Type of a printer function, used by mdcs_set_error_printer
 * and mdcs_set_warning_printer.
 */
typedef void (*mdcs_printer_f)(const char*);

/**
 * Initializes the MDCS service by registering the proper
 * RPCs using the provided margo instance.
 *
 * \param[in] mid Initialized margo instance.
 * \param[in] listening MDCS_TRUE if we are listening for queries.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_init(margo_instance_id mid, int listening);

int mdcs_initialized(int* flag);

/**
 * Finalizes the MDCS service.
 *
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_finalize();

/**
 * Sets the function to use to print errors inside MDCS.
 * 
 * \param[in] fun Function to call to print an error message.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_set_error_printer(mdcs_printer_f fun);

/**
 * Sets the function to use to print warnings inside MDCS.
 * 
 * \param[in] fun Function to call to print a warning message.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_set_warning_printer(mdcs_printer_f fun);

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
 * Registers a new counter. Will fail if the name of the
 * counter already exists.
 *
 * \param[in] name Name of the counter.
 * \param[in] type Type of counter.
 * \param[in] buffer_size Size of the buffer to cache counter values (can be 0).
 * \param[out] counter Newly created counter.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_counter_register(const char* name,
        mdcs_counter_type_t type, size_t buffer_size, 
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

/**
 * Finds a counter by its id.
 *
 * \param[in] id ID of the counter to find.
 * \param[out] counter Returned counter.
 * \return MDCS_SUCCESS if counter is found, MDCS_ERROR otherwise.
 */
int mdcs_counter_find_by_id(uint64_t id, mdcs_counter_t* counter);

/**
 * Finds a counter by its name. 
 * 
 * \param[in] name Name of the counter to find.
 * \param[out] counter Returned counter.
 * \return MDCS_SUCCESS if counter is found, MDCS_ERROR otherwise.
 */
int mdcs_counter_find_by_name(const char* name, mdcs_counter_t* counter);

/**
 * Gets the id of a counter in a given namespace.
 * 
 * \param[in] name Name of the counter.
 * \param[out] counter Resulting counter id object..
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_remote_counter_get_id(const char* name, mdcs_counter_id_t* counter);

/**
 * Fetches the value of a counter from a remote address.
 * 
 * \param[in] addr Server address from which to fetch the counter value.
 * \param[in] counter Counter from which to fetch the value.
 * \param[out] value Pointer to a buffer where to store the value.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_remote_counter_fetch(hg_addr_t addr, mdcs_counter_id_t counter, void* value, size_t size);

/**
 * Resets a counter at a remote address.
 * 
 * \param[in] addr Address of the server in which to reset the counter.
 * \param[in] counter Counter to reset.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_remote_counter_reset(hg_addr_t addr, mdcs_counter_id_t counter);

#ifdef __cplusplus
}
#endif

#endif
