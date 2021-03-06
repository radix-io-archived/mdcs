#include <assert.h>
#include <stdio.h>
#include <margo.h>
#include <mdcs/mdcs.h>
#include <mdcs/mdcs-counters.h>
#include "range-tracker.h"
#include "types.h"

/* Main function. */
int main(int argc, char** argv)
{
	/* Start Margo */
	margo_instance_id mid = margo_init("bmi+tcp", MARGO_CLIENT_MODE, 0, 0);

	/* Register a RPC function */
	hg_id_t sum_rpc_id = MARGO_REGISTER(mid, "sum", sum_in_t, sum_out_t, NULL);

	mdcs_init(mid, MDCS_FALSE, ABT_POOL_NULL);
	
	/* Lookup the address of the server */
	hg_addr_t svr_addr;
	margo_addr_lookup(mid, "bmi+tcp://localhost:1234", &svr_addr);

	mdcs_counter_id_t cid1, cid2, cid3;
	mdcs_remote_counter_get_id("example:mycounter", &cid1);
	mdcs_remote_counter_get_id("example:mystats", &cid2);
	mdcs_remote_counter_get_id("example:myrange", &cid3);

	int i;
	sum_in_t args;
	for(i=0; i<4; i++) {
		args.x = 42+i*2;
		args.y = 42+i*2+1;

		hg_handle_t h;
		margo_create(mid, svr_addr, sum_rpc_id, &h);
		margo_forward(h, &args);
		
		sum_out_t resp;
		margo_get_output(h, &resp);

		printf("Got response: %d+%d = %d\n", args.x, args.y, resp.ret);

		int64_t counter_value;
		mdcs_remote_counter_fetch(svr_addr, cid1, &counter_value, sizeof(counter_value));
		mdcs_counter_stat_double_value_t stats;
		mdcs_remote_counter_fetch(svr_addr, cid2, &stats, sizeof(stats));
		range_tracker_value_t range;
		mdcs_remote_counter_fetch(svr_addr, cid3, &range, sizeof(range));

		printf("Counter value is %ld\n", counter_value);
		printf("Stats: count=%ld min=%lf, max=%lf, avg=%lf, var=%lf, last=%lf\n",
			stats.count, stats.min, stats.max, stats.avg, stats.var, stats.last);
		printf("Range value is %d\n", range);

		margo_free_output(h,&resp);
		margo_destroy(h);
	}

	/* free the address */
	margo_addr_free(mid, svr_addr);

	mdcs_finalize();

	/* shut down Margo */
    margo_finalize(mid);

	return 0;
}
