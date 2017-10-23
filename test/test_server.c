#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <margo.h>
#include <mdcs/mdcs.h>
#include <mdcs/mdcs-counters.h>
#include "types.h"
#include "range-tracker.h"

static mdcs_counter_t mycounter = MDCS_COUNTER_NULL;
static mdcs_counter_t mystats   = MDCS_COUNTER_NULL;
static mdcs_counter_t myrange   = MDCS_COUNTER_NULL;

/* 
 * hello_world function to expose as an RPC.
 * This function just prints "Hello World"
 * and increment the num_rpcs variable.
 *
 * All Mercury RPCs must have a signature
 *   hg_return_t f(hg_handle_t h)
 */
hg_return_t sum(hg_handle_t h);
DECLARE_MARGO_RPC_HANDLER(sum)

/*
 * main function.
 */
int main(int argc, char** argv)
{
	srand(time(NULL));

	/* Initialize Margo */
	margo_instance_id mid = margo_init("bmi+tcp://localhost:1234", MARGO_SERVER_MODE, 0, 0);
    assert(mid);

	/* Register the RPC by its name ("sum") */
	MARGO_REGISTER(mid, "sum", sum_in_t, sum_out_t, sum);

	int ret;
	
	ret = mdcs_init(mid, MDCS_TRUE, ABT_POOL_NULL);
	assert(ret == MDCS_SUCCESS);

	mdcs_counter_type_t range_tracker_type = MDCS_COUNTER_TYPE_NULL;

	mdcs_counter_type_create(sizeof(range_tracker_item_t),
	                         sizeof(range_tracker_value_t),
                             (mdcs_create_f)range_tracker_create,
                             (mdcs_destroy_f)range_tracker_destroy,
	                         (mdcs_reset_f)range_tracker_reset,
	                         (mdcs_push_one_f)range_tracker_push_one,
	                         (mdcs_push_multi_f)range_tracker_push_multi,
	                         (mdcs_get_value_f)range_tracker_get_value,
	                         &range_tracker_type);

	mdcs_counter_register("example:myrange", range_tracker_type, 0, &myrange);
	mdcs_counter_register("example:mycounter", MDCS_COUNTER_LAST_INT64, 0, &mycounter); 
	mdcs_counter_register("example:mystats", MDCS_COUNTER_STAT_DOUBLE, 0, &mystats);

	margo_wait_for_finalize(mid);

	mdcs_counter_type_destroy(range_tracker_type);

	mdcs_finalize();

	return 0;
}

/* Implementation of the RPC. */
hg_return_t sum(hg_handle_t h)
{
	hg_return_t ret;

	sum_in_t in;
	sum_out_t out;

	/* Deserialize the input from the received handle. */
	ret = margo_get_input(h, &in);
	assert(ret == HG_SUCCESS);

	/* Compute the result. */
	out.ret = in.x + in.y;
	printf("Computed %d + %d = %d\n",in.x,in.y,out.ret);

	int64_t v = out.ret;
	int r;
	r = mdcs_counter_push(mycounter, &v);
	assert(r == MDCS_SUCCESS);
	int64_t stored = 0;
	r = mdcs_counter_value(mycounter, &stored);

	int32_t v1 = in.x;
	int32_t v2 = in.y;
	int32_t v3 = out.ret;
	mdcs_counter_push(myrange, &v1);
	mdcs_counter_push(myrange, &v2);
	mdcs_counter_push(myrange, &v3);

	int i;
	for(i=0; i<20; i++) {
		double random_value;
		random_value = (double)rand()/RAND_MAX*2.0-1.0; //float in range -1 to 1
		mdcs_counter_push(mystats, &random_value);
	}

	r = mdcs_counter_value(mycounter, &stored);
    printf("Stored counter value is %ld\n", stored);

	ret = margo_respond(h, &out);
	assert(ret == HG_SUCCESS);

	/* Free the input data. */
	ret = margo_free_input(h, &in);
	assert(ret == HG_SUCCESS);

	/* We are not going to use the handle anymore, so we should destroy it. */
	ret = margo_destroy(h);
	assert(ret == HG_SUCCESS);

	return HG_SUCCESS;
}
DEFINE_MARGO_RPC_HANDLER(sum)
