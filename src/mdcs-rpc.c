/*
 * Copyright (c) 2017 UChicago Argonne, LLC
 *
 * See COPYRIGHT in top-level directory.
 */
#include "mdcs-rpc.h"
#include "mdcs-rpc-types.h"
#include "mdcs-global-data.h"
#include "mdcs-error.h"

extern mdcs_t g_mdcs;

hg_return_t mdcs_rpc_get_counter(hg_handle_t handle)
{
	hg_return_t result = HG_SUCCESS;
	int ret = HG_SUCCESS;
	const struct hg_info* info = NULL;
	margo_instance_id mid = MARGO_INSTANCE_NULL;
	fetch_counter_in_t in = {
		.counter_id = 0,
		.size = 0,
		.bulk_handle = HG_BULK_NULL
	};
	fetch_counter_out_t out = {
		.ret = MDCS_SUCCESS
	};
	mdcs_counter_t counter = MDCS_COUNTER_NULL;
	hg_bulk_t bulk_handle = HG_BULK_NULL;
	void* buffer = NULL;

	mid = margo_hg_handle_get_instance(handle);
	if(MARGO_INSTANCE_NULL == mid) {
		MDCS_PRINT_ERROR("Could not get a valid Margo instance");
		result = HG_OTHER_ERROR;
		goto cleanup;
	}

	info = margo_get_info(handle);
	if(!info) {
		MDCS_PRINT_ERROR("Could not get info from handle");
		result = HG_OTHER_ERROR;
		goto cleanup;
	}

	ret = margo_get_input(handle, &in);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_ERROR("Could not get input from handle");
		result = ret;
		goto cleanup;
	}

	ret = mdcs_counter_find_by_id(in.counter_id, &counter);
	if(ret == MDCS_ERROR) {

		out.ret = MDCS_ERROR;

	} else {

		buffer = calloc(1,in.size);
		if(buffer == NULL) {
			MDCS_PRINT_ERROR("Could not allocate buffer");
			result = HG_OTHER_ERROR;
			goto cleanup;
		}

		ret = mdcs_counter_value(counter, buffer);
		if(ret != MDCS_SUCCESS) {
			MDCS_PRINT_ERROR("Could not get counter value");
			result = HG_OTHER_ERROR;
			goto cleanup;
		}

		ret = margo_bulk_create(mid, 1, &buffer,
				&in.size, HG_BULK_READ_ONLY, &bulk_handle);
		if(ret != HG_SUCCESS) {
			MDCS_PRINT_ERROR("Could create bulk handle");
			result = ret;
			goto cleanup;
		}

    	ret = margo_bulk_transfer(mid, HG_BULK_PUSH,
				info->addr, in.bulk_handle, 0,
				bulk_handle, 0, in.size);
		if(ret != HG_SUCCESS) {
			MDCS_PRINT_ERROR("Could not issue bulk transfer");
			result = ret;
			goto cleanup;
		}
	}

	ret = margo_respond(mid, handle, &out);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_ERROR("Could not respond to RPC");
		result = ret;
		goto cleanup;
	}

cleanup:

	free(buffer);

	ret = margo_free_input(handle, &in);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_WARNING("Could not free input");
		result = ret;
	}

	ret = margo_bulk_free(bulk_handle);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_WARNING("Could not free bulk handle");
		result = ret;
	}

	ret = margo_destroy(handle);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_WARNING("Could not destroy RPC handle");
		result = ret;
	}

	return result;
}
DEFINE_MARGO_RPC_HANDLER(mdcs_rpc_get_counter)

hg_return_t mdcs_rpc_reset_counter(hg_handle_t handle)
{
	hg_return_t result = HG_SUCCESS;
	int ret = HG_SUCCESS;
	margo_instance_id mid = MARGO_INSTANCE_NULL; 
	reset_counter_in_t  in = {
		.counter_id = 0
	};
	reset_counter_out_t out = {
		.ret = MDCS_SUCCESS
	};	
	mdcs_counter_t counter = MDCS_COUNTER_NULL;

	mid = margo_hg_handle_get_instance(handle);
	if(mid == MARGO_INSTANCE_NULL) {
		MDCS_PRINT_ERROR("Could not get a valid Margo instance from handle");
		result = HG_OTHER_ERROR;
		goto cleanup;
	}

	ret = mdcs_counter_find_by_id(in.counter_id, &counter);

	if(ret == MDCS_SUCCESS)
		ret = mdcs_counter_reset(counter);
	
	out.ret = ret;

	ret = margo_respond(mid, handle, &out);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_ERROR("Could not send RPC response");
		result = ret;
		goto cleanup;
	}

cleanup:

	ret = margo_free_input(handle, &in);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_WARNING("Could not free input");
		result = ret;
	}

	ret = margo_destroy(handle);
	if(ret != HG_SUCCESS) {
		MDCS_PRINT_WARNING("Could not destroy RPC handle");
		result = ret;
	}

	return result;
}
DEFINE_MARGO_RPC_HANDLER(mdcs_rpc_reset_counter)
