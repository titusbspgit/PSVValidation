// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * File: test_define.inc
 * Test Case Name: ethernet1_tx_basic_test
 * Purpose: Testcase supporting data for Ethernet1 basic TX DMA packet
 *          transmission using interrupt-driven completion.
 */

#include "test.h"

/* Meta TestPlan Headers */
#include <stdio.h>
#include <stdlib.h>
#include <ethernet1/ethernet1_def.h>
#include <ethernet1/ethernet1_offset.h>
#include <test_common.h>
#include <ethernet0/ethernet0_programming_sequence.h>

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* ---- Macros ---- */
#define HSS_AUTOREG_INT_ENABLE  0xE68C2058U
#define HSS_AUTOREG_INT_CLEAR   0xE68C2050U

#define ETH1_TX_IRQ_LOOP_COUNT  10U

/*
 * Timeout for polling int_pend in the TX interrupt loop.
 * MANUAL_REVIEW: Adjust this value based on platform timing requirements.
 */
#define ETH1_TX_IRQ_TIMEOUT     100000U

/* ---- External Variables ---- */
extern volatile int int_pend;

/* ---- Arrays ---- */
// Arrays field is NA for this testcase. No arrays to declare.
