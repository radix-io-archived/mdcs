#ifndef __MDCS_DATA_H
#define __MDCS_DATA_H

#include <mdcs/mdcs.h>

typedef struct mdcs_data_s {
    mdcs_counter_t counter_hash;
	margo_instance_id mid;
	hg_id_t rpc_fetch_id;
	hg_id_t rpc_reset_id;
}* mdcs_t;

#define MDCS_NULL ((mdcs_t)NULL)

#endif
