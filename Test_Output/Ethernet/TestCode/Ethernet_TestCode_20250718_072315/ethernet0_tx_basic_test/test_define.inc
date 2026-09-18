// Author - AI Force 2.3. 18-Jul-2025 07:23 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: ethernet0_tx_basic_test
 * Purpose: Verifies basic Ethernet TX packet transmission on Ethernet0 using
 *   interrupt-driven DMA transfers. Configures MAC, MTL, and DMA layers,
 *   preloads TX descriptors, starts TX on CH0, and waits for transfer-complete
 *   interrupts.
 */

#include "test.h"

// MANUAL_REVIEW: MetaData sheet had no Headers for this testcase.
// The following headers are inferred from the testcase IP (Ethernet0).
// Verify and add any additional required headers.
#include <stdio.h>
#include <stdlib.h>
#include <ethernet0/ethernet0_def.h>
#include <test_common.h>
#include <ethernet0/ethernet0_offset.h>

// MANUAL_REVIEW: MetaData sheet had no Macros for this testcase.
// The following macros are derived from TestPlan sheet context.

/* Speed selection compile-time defines */
// MANUAL_REVIEW: Define exactly one speed macro to select the test speed.
// #define ETH_10M
// #define ETH_100M
// Default is 1G if neither is defined.

/* Duplex mode selection */
// MANUAL_REVIEW: Define HALF_DUPLEX for half-duplex mode.
// Default is full duplex if not defined.
// #define HALF_DUPLEX

/* TX descriptor ring length (10 descriptors per channel) */
#define ETH0_TX_DESC_RING_LEN   10U

/* Number of DMA channels */
#define ETH0_DMA_CHANNEL_CNT    4U

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

// MANUAL_REVIEW: MetaData sheet had no Arrays for this testcase.
// No arrays to generate.
