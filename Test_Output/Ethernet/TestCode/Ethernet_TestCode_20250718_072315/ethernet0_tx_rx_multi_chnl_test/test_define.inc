// Author - AI Force 2.3. 18-Jul-2025 07:23 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: ethernet0_tx_rx_multi_chnl_test
 * Purpose: Verifies multi-channel TX and RX Ethernet packet transfer on
 *   Ethernet0 using all four DMA channels (CH0-CH3) in a sequential phased
 *   approach. Phase 1 activates CH3, Phase 2 CH2, Phase 3 CH1, Phase 4 CH0.
 *   Each phase transfers 10 packets for a cumulative total of 40 TX and 40 RX.
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

/* Number of DMA channels */
#define ETH0_DMA_CHANNEL_CNT    4U

/* TX descriptor ring length (10 descriptors per channel) */
#define ETH0_TX_DESC_RING_LEN   10U

/* Packets per phase */
#define ETH0_PHASE_PKT_CNT     10U

/* Total expected packets across all 4 phases */
#define ETH0_TOTAL_PKT_CNT     40U

/* MTL TX queue sizes */
#define ETH0_MTL_TX_QUEUE_SIZE  1024U  /* 1KB for TX queues */
#define ETH0_MTL_RX_QUEUE_SIZE  4096U  /* 4096B for RX queues */

/* DMA burst length in beats */
#define ETH0_DMA_BURST_LEN      16U

/* Phase polling iteration limit (20 iterations per phase as per test description) */
#ifndef ETH0_PHASE_POLL_LIMIT
#define ETH0_PHASE_POLL_LIMIT   20U
#endif

/* VIP sequencer trigger values for each phase */
#define ETH0_VIP_TRIGGER_PHASE1 0xDEADBEEFU
#define ETH0_VIP_TRIGGER_PHASE2 0xDEADBEEEU
#define ETH0_VIP_TRIGGER_PHASE3 0xDEADBEEDU
#define ETH0_VIP_TRIGGER_PHASE4 0xDEADBEECU

/* TX quantum weights */
#define ETH0_TXQ_WEIGHT_HIGH    20U  /* Q3 and Q2 */
#define ETH0_TXQ_WEIGHT_LOW     5U   /* Q1 and Q0 */

// MANUAL_REVIEW: MetaData sheet had no Arrays for this testcase.
// No arrays to generate.
