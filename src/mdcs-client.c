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
#include "mdcs-error.h"

extern mdcs_t g_mdcs;

int mdcs_remote_counter_get_id(const char* name, mdcs_counter_id_t* counter)
{
	*counter = mdcs_hash_string(name);
	return MDCS_SUCCESS;
}

int mdcs_remote_counter_fetch(hg_addr_t addr, mdcs_counter_id_t counter, void* value, size_t size)
{
	int result = MDCS_SUCCESS;
	hg_return_t ret = HG_SUCCESS;
	hg_handle_t handle = HG_HANDLE_NULL;

	fetch_counter_in_t in = {
		.counter_id = counter,
		.size = size,
		.bulk_handle = HG_BULK_NULL
	};
	fetch_counter_out_t out = {
		.ret = MDCS_SUCCESS
	};

	ret = margo_create(g_mdcs->mid, addr, g_mdcs->rpc_fetch_id, &handle);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_ERROR("Could not create RPC handle");
		result = MDCS_ERROR;
		goto cleanup;
	}

	ret = margo_bulk_create(g_mdcs->mid, 1, &value, &size,
                    HG_BULK_WRITE_ONLY, &(in.bulk_handle));
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_ERROR("Could not create bulk handle");
		result = MDCS_ERROR;
		goto cleanup;
	}

	ret = margo_forward(g_mdcs->mid, handle, &in);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_ERROR("Count not forward RPC");
		result = MDCS_ERROR;
		goto cleanup;
	}	

	ret = margo_get_output(handle, &out);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_ERROR("Could not get RPC output");
		result = MDCS_ERROR;
		goto cleanup;
	}

cleanup:

	ret = margo_bulk_free(in.bulk_handle);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_WARNING("Could not free RPC handle");
	}

	ret = margo_free_output(handle, &out);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_WARNING("Coult not free RPC output");
	}

	ret = margo_destroy(g_mdcs->mid, handle);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_WARNING("Could not destroy RPC handle");
	}

	return result;
}

int mdcs_remote_counter_reset(hg_addr_t addr, mdcs_counter_id_t counter)
{
	int result = MDCS_SUCCESS;
	hg_return_t ret = HG_SUCCESS;
	hg_handle_t handle = HG_HANDLE_NULL;

	reset_counter_in_t in = {
		.counter_id = counter
	};
	reset_counter_out_t out = {
		.ret = MDCS_SUCCESS
	};

	ret = margo_create(g_mdcs->mid, addr, g_mdcs->rpc_reset_id, &handle);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_ERROR("Could not create RPC handle");
		result = MDCS_ERROR;
		goto cleanup;
	}

	ret = margo_forward(g_mdcs->mid, handle, &in);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_ERROR("Could not forward RPC");
		result = MDCS_ERROR;
		goto cleanup;
	}
	
	ret = margo_get_output(handle, &out);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_ERROR("Could not get RPC output");
		result = MDCS_ERROR;
		goto cleanup;
	}

cleanup:

	ret = margo_free_output(handle, &out);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_WARNING("Could not free output");
	}

	ret = margo_destroy(g_mdcs->mid, handle);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_WARNING("Could not free RPC handle");
	}

	return result;
}
