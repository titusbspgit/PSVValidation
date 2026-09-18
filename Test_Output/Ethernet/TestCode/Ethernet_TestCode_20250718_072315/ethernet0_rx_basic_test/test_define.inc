// Author - AI Force 2.3. 18-Jul-2025 07:23 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: ethernet0_rx_basic_test
 * Purpose: Verifies basic Ethernet RX packet reception on Ethernet0 using
 *   interrupt-driven DMA transfers. Configures MAC, MTL, and DMA layers
 *   across all four channels (CH0-CH3), preloads RX descriptors, starts
 *   the RX path, triggers an external VIP sequencer, and waits for
 *   transfer-complete interrupts.
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

/* Macros from Meta TestPlan */
// MANUAL_REVIEW: ETH_10M macro name was provided but no value was supplied.
// Define to select 10M speed mode. Uncomment if needed.
// #define ETH_10M

// MANUAL_REVIEW: ETH_100M macro name was provided but no value was supplied.
// Define to select 100M speed mode. Uncomment if needed.
// #define ETH_100M

// MANUAL_REVIEW: HALF_DUPLEX macro name was provided but no value was supplied.
// Define to select half-duplex mode. Uncomment if needed.
// #define HALF_DUPLEX

// MANUAL_REVIEW: SEL_ENET0 macro name was provided but no value was supplied.
// Define the correct value for Ethernet0 interface selection.
#ifndef SEL_ENET0
#define SEL_ENET0 0U
#endif

// MANUAL_REVIEW: SEL_ENET1 macro name was provided but no value was supplied.
#ifndef SEL_ENET1
#define SEL_ENET1 1U
#endif

// MANUAL_REVIEW: SEL_ENET2 macro name was provided but no value was supplied.
#ifndef SEL_ENET2
#define SEL_ENET2 2U
#endif

// MANUAL_REVIEW: SEL_ENET3 macro name was provided but no value was supplied.
#ifndef SEL_ENET3
#define SEL_ENET3 3U
#endif

// MANUAL_REVIEW: DEBUG_DISPLAY macro name was provided but no value was supplied.
// Do not redefine if already provided by the framework.
// #define DEBUG_DISPLAY

/* Testcase-specific constants */

/* DMA descriptor ring length (32 descriptors per channel) */
#define ETH0_RX_DESC_RING_LEN   32U

/* Number of DMA channels */
#define ETH0_DMA_CHANNEL_CNT    4U

/* MTL queue size in bytes */
#define ETH0_MTL_QUEUE_SIZE     4096U

/* DMA burst length in beats */
#define ETH0_DMA_BURST_LEN      16U

/* Expected number of transfer-complete interrupts (10 default, 6 for 10M) */
#ifdef ETH_10M
#define ETH0_EXPECTED_TRANSFER_CNT 6U
#else
#define ETH0_EXPECTED_TRANSFER_CNT 10U
#endif

/* Polling timeout for interrupt wait loop */
// MANUAL_REVIEW: Set ETH0_POLL_TIMEOUT to an appropriate value for the target platform.
#ifndef ETH0_POLL_TIMEOUT
#define ETH0_POLL_TIMEOUT 1000000U
#endif

// Arrays field was NA - no arrays to generate.
