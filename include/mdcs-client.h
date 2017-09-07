#ifndef __MOCHI_MDCS_CLIENT_H
#define __MOCHI_MDCS_CLIENT_H

#include <mdcs/mdcs.h>

typedef struct mdcs_counter_id_s* mdcs_counter_id_t;
typedef struct mdcs_namespace_id_s* mdcs_namespace_id_t;
typedef struct mdcs_namespace_list_s* mdcs_namespace_list_t;
typedef struct mdcs_counter_list_s* mdcs_counter_list_t;

/* The following structures are placed here for reference 
   but their implementation is located in mdcs-client.c

struct mdcs_namespace_id_s {
	uint64_t id; // id of the namespace
	char* name;  // name of the namespace
};

struct mdcs_namespace_list_s {
	size_t num_namespaces;            // number of stored namespace references
	mdcs_namespace_id_t namespace[1]; // array of namepace references
};

struct mdcs_counter_id_s {
	uint64_t id; // id of the counter in the remote server
};

struct mdcs_counter_list_s {
	size_t num_counters;          // number of stored counter references
	mdcs_counter_id_t counter[1]; // array of counter references
};
*/

/**
 * Fetches from an address the list of available namespaces at that address.
 * The resulting namespace list should be freed with mdcs_namespaces_free.
 *
 * \param[in] server_addr Address of the server to contact.
 * \param[out] nd_list Namespace list object.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_namespaces_fetch(hg_addr_t server_addr, mdcs_namespace_list_t* ns_list);

/**
 * Frees the list of namespaces created by mdcs_namespaces_fetch.
 * After freeing a list of namespace, mdcs_namespace_id_t objects obtained
 * from that list using mdcs_namespaces_get become invalid and should not be used.
 *
 * \param[in] ns_list Namespace list to free.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_namespaces_free(mdcs_namespace_list_t ns_list);

/**
 * Counts the number of namespaces present in a namespace list.
 * 
 * \param[in] ns_list List of namespaces.
 * \param[out] count Number of namespaces.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_namespaces_count(mdcs_namespace_list_t ns_list, size_t* count);

/**
 * Gets a particular namespace from the list.
 * 
 * \param[in] ns_list Namespace list from which to take a namespace.
 * \param[in] index Index of the namespace to retrieve.
 * \param[out] namespace Namespace returned. 
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_namespaces_get(mdcs_namespace_list_t ns_list, size_t index, mdcs_namespace_id_t* namespace);

/**
 * Gets the name of a namespace.
 * 
 * \param[in] namespace Namespace from which to get the name.
 * \param[out] name Name of the namespace.
 * \param[in] bufsize Maximum size of the name buffer.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_namespace_get_name(mdcs_namespace_id_t namespace, char* name, size_t bufsize);

/**
 * Lists the counter associated with a namespace. The resulting list of counters
 * should be freed with mdcs_counters_free.
 * 
 * \param[in] namespace Namespace from which to list the counters.
 * \param[out] cnt_list Resulting list of counters.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_namespace_list_counters(mdcs_namespace_id_t namespace, mdcs_counter_list_t* cnt_list);

/**
 * Frees a list of counters. After freeing a list of counters, counters obtained
 * using mdcs_counters_get from that list should not be used anymore.
 * 
 * \param[in] cnt_list List of counters to free.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_counters_free(mdcs_counter_list_t cnt_list);

/**
 * Counts the number of counters within a list.
 * 
 * \param[in] cnt_list List of counters.
 * \param[out] size Number of counters in the list.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_counters_count(mdcs_counter_list_t cnt_list, size_t* size);

/**
 * Gets a particular counter from a given list of counter.
 * 
 * \param[in] cnt_list List of counters.
 * \param[in] index Index of the counter to retrieve.
 * \param[out] counter Retrieved counter.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_counters_get(mdcs_counter_list_t cnt_list, size_t index, mdcs_counter_id_t* counter);

/**
 * Fetches the value of a counter from a remote address.
 * 
 * \param[in] addr Server address from which to fetch the counter value.
 * \param[in] counter Counter from which to fetch the value.
 * \param[out] value Pointer to a buffer where to store the value.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_counter_fetch_value(hg_addr_t addr, mdcs_counter_id_t counter, void* value);

/**
 * Resets a counter at a remote address.
 * 
 * \param[in] addr Address of the server in which to reset the counter.
 * \param[in] counter Counter to reset.
 * \return MDCS_SUCCESS on success, MDCS_ERROR otherwise.
 */
int mdcs_counter_reset(hg_addr_t addr, mdcs_counter_id_t counter);

#endif