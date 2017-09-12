#ifndef __MDCS_RPC_TYPES_H
#define __MDCS_RPC_TYPES_H

#include <mercury.h>
#include <mercury_bulk.h>
#include <mercury_types.h>
#include <mercury_proc_string.h>
#include <mercury_macros.h>

MERCURY_GEN_PROC(get_counter_in_t,
    ((uint64_t)(counter_id))\
	((uint64_t)(size))\
    ((hg_bulk_t)(bulk_handle)))

MERCURY_GEN_PROC(get_counter_out_t, ((int32_t)(ret)))

MERCURY_GEN_PROC(reset_counter_in_t,
	((uint64_t)(counter_id)))

MERCURY_GEN_PROC(reset_counter_out_t, ((int32_t)(ret)))

#endif
