// Author - AI Force 2.3. 21-Sep-2025 20:45 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case Name: ethernet2_tx_basic_test
 * Purpose: Basic Ethernet2 TX transmission using DMA channel 0 with
 *          interrupt-driven transfer completion across multiple channels.
 */

#include "test.h"

/* Meta TestPlan Headers */
#include <stdio.h>
#include <stdlib.h>
#include <test_common.h>
#include <ethernet2/ethernet2_def.h>
#include <ethernet2/ethernet2_offset.h>
#include <ethernet0/ethernet0_programming_sequence.h>

/*
 * Timeout counter for interrupt polling loop.
 * MANUAL_REVIEW: Adjust this value based on platform timing requirements.
 */
#define ETH2_TX_IRQ_TIMEOUT 100000U
