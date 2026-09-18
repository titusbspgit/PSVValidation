// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: ethernet0_rx_basic_test
 * Purpose:   Basic RX packet reception test for Ethernet0.
 *            Test Description was NA. This file contains headers and macros
 *            from the Meta TestPlan JSON.
 */

#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ethernet0/ethernet0_def.h>
#include <test_common.h>
#include <ethernet0/ethernet0_offset.h>
#include <ethernet0/ethernet0_funcs.h>
#include <ethernet0/ethernet0_programming_sequence.h>

/*
 * Macros from Meta TestPlan JSON.
 * MANUAL_REVIEW: Exact values for ETH_10M, ETH_100M, HALF_DUPLEX,
 *                SEL_ENET0, SEL_ENET1, SEL_ENET2, SEL_ENET3, and
 *                DEBUG_DISPLAY were not provided in the Meta TestPlan JSON.
 *                Populate these with the correct SoC/IP-specific values.
 */
#ifndef ETH_10M
#define ETH_10M          0U  /* MANUAL_REVIEW: Set correct value for 10M speed selection */
#endif

#ifndef ETH_100M
#define ETH_100M         0U  /* MANUAL_REVIEW: Set correct value for 100M speed selection */
#endif

#ifndef HALF_DUPLEX
#define HALF_DUPLEX      0U  /* MANUAL_REVIEW: Set correct value for half duplex mode */
#endif

#ifndef SEL_ENET0
#define SEL_ENET0        0U  /* MANUAL_REVIEW: Set correct value for Ethernet0 selection */
#endif

#ifndef SEL_ENET1
#define SEL_ENET1        0U  /* MANUAL_REVIEW: Set correct value for Ethernet1 selection */
#endif

#ifndef SEL_ENET2
#define SEL_ENET2        0U  /* MANUAL_REVIEW: Set correct value for Ethernet2 selection */
#endif

#ifndef SEL_ENET3
#define SEL_ENET3        0U  /* MANUAL_REVIEW: Set correct value for Ethernet3 selection */
#endif

#ifndef DEBUG_DISPLAY
#define DEBUG_DISPLAY    0U  /* MANUAL_REVIEW: Set to 1 to enable debug display output */
#endif

/* Arrays: NA in Meta TestPlan JSON. No arrays generated. */
