#ifndef __MOCHI_MDCS_CLIENT_H
#define __MOCHI_MDCS_CLIENT_H

#include <mdcs/mdcs.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint64_t mdcs_counter_id_t;

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
int mdcs_remote_counter_fetch(hg_addr_t addr, mdcs_counter_id_t counter, void* value);

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
