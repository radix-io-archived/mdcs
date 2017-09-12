#include "mdcs-rpc.h"
#include "mdcs-rpc-types.h"
#include "mdcs-global-data.h"

extern mdcs_t g_mdcs;

hg_return_t mdcs_rpc_get_counter(hg_handle_t handle)
{
	margo_instance_id mid = margo_hg_handle_get_instance(handle);
	const struct hg_info* info = margo_get_info(handle);

	get_counter_in_t in;
	get_counter_out_t out;
	
	margo_get_input(handle, &in);
	
	mdcs_counter_t counter;
	int ret = mdcs_counter_find_by_id(in.counter_id, &counter);
	if(ret == MDCS_ERROR) {
		out.ret = MDCS_ERROR;
	} else {

		void* buffer = calloc(1,in.size);
		hg_bulk_t bulk_handle;

		mdcs_counter_value(counter, buffer);

		margo_bulk_create(mid, 1, &buffer,
				&in.size, HG_BULK_READ_ONLY, &bulk_handle);

    	margo_bulk_transfer(mid, HG_BULK_PUSH,
				info->addr, in.bulk_handle, 0,
				bulk_handle, 0, in.size);
		free(buffer);
		out.ret = MDCS_SUCCESS;
	}

	margo_respond(mid, handle, &out);
		
	margo_free_input(handle, &in);
	margo_destroy(handle);

	return HG_SUCCESS;
}
DEFINE_MARGO_RPC_HANDLER(mdcs_rpc_get_counter)

hg_return_t mdcs_rpc_reset_counter(hg_handle_t handle)
{
	int ret;
	margo_instance_id mid = margo_hg_handle_get_instance(handle);
	reset_counter_in_t  in;
	reset_counter_out_t out;
	
	mdcs_counter_t counter;
	ret = mdcs_counter_find_by_id(in.counter_id, &counter);

	if(ret == MDCS_SUCCESS)
		ret = mdcs_counter_reset(counter);
	
	out.ret = ret;

	margo_respond(mid, handle, &out);
	margo_free_input(handle, &in);
	margo_destroy(handle);

	return HG_SUCCESS;
}
DEFINE_MARGO_RPC_HANDLER(mdcs_rpc_reset_counter)
