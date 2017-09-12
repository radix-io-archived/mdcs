/*
 * Copyright (c) 2017 UChicago Argonne, LLC
 *
 * See COPYRIGHT in top-level directory.
 */
#include <assert.h>
#include <mdcs/mdcs.h>
#include "mdcs-global-data.h"
#include "mdcs-rpc-types.h"
#include "mdcs-rpc.h"
#include "mdcs-hash-string.h"

extern mdcs_t g_mdcs;

int mdcs_remote_counter_get_id(const char* name, mdcs_counter_id_t* counter)
{
	*counter = mdcs_hash_string(name);
	return MDCS_SUCCESS;
}

int mdcs_remote_counter_fetch(hg_addr_t addr, mdcs_counter_id_t counter, void* value, size_t size)
{
	hg_return_t ret;
	hg_handle_t handle;

	get_counter_in_t in;
	get_counter_out_t out;

	ret = margo_create(g_mdcs->mid, addr, g_mdcs->rpc_fetch_id, &handle);
	assert(ret == HG_SUCCESS);

	in.counter_id = counter;
	in.size = size;
	ret = margo_bulk_create(g_mdcs->mid, 1, &value, &size,
                    HG_BULK_WRITE_ONLY, &(in.bulk_handle));
    assert(ret == HG_SUCCESS);

	margo_forward(g_mdcs->mid, handle, &in);
	
	ret = margo_get_output(handle, &out);
	assert(ret == HG_SUCCESS);

	ret = margo_bulk_free(in.bulk_handle);
	assert(ret == HG_SUCCESS);

	ret = margo_free_output(handle, &out);
	assert(ret == HG_SUCCESS);

	ret = margo_destroy(handle);
	assert(ret == HG_SUCCESS);

	return MDCS_SUCCESS;
}

int mdcs_remote_counter_reset(hg_addr_t addr, mdcs_counter_id_t counter)
{
	hg_return_t ret;
	hg_handle_t handle;

	reset_counter_in_t in;
	reset_counter_out_t out;

	ret = margo_create(g_mdcs->mid, addr, g_mdcs->rpc_reset_id, &handle);
	assert(ret == HG_SUCCESS);

	in.counter_id = counter;

	margo_forward(g_mdcs->mid, handle, &in);
	
	ret = margo_get_output(handle, &out);
	assert(ret == HG_SUCCESS);

	ret = margo_free_output(handle, &out);
	assert(ret == HG_SUCCESS);

	ret = margo_destroy(handle);
	assert(ret == HG_SUCCESS);

	return MDCS_SUCCESS;
}
