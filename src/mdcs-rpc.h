#ifndef __MDCS_RPC_H
#define __MDCS_RPC_H

#include <margo.h>

hg_return_t mdcs_rpc_get_counter(hg_handle_t handle);
DECLARE_MARGO_RPC_HANDLER(mdcs_rpc_get_counter);

hg_return_t mdcs_rpc_reset_counter(hg_handle_t handle);
DECLARE_MARGO_RPC_HANDLER(mdcs_rpc_reset_counter);

#endif
