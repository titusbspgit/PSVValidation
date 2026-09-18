// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: ethernet0_tx_rx_multi_chnl_test
 * Purpose:   Multi-channel Ethernet TX and RX data path validation.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <test_common.h>
#include <ethernet0/ethernet0_def.h>
#include <ethernet0/ethernet0_offset.h>
#include <ethernet0/ethernet0_funcs.h>
#include <ethernet0/ethernet0_programming_sequence.h>

#ifndef ETHERNET0_MULTI_CHNL_TIMEOUT
#define ETHERNET0_MULTI_CHNL_TIMEOUT 500000U
#endif
