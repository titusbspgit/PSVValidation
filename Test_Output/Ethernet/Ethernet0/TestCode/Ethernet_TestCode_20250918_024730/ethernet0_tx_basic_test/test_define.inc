// Author - AI Force 2.3. 18-Sep-2025 08:17 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: ethernet0_tx_basic_test
 * Purpose:   Verifies basic Ethernet TX packet transmission on Ethernet0 using
 *            interrupt-driven DMA transfers across four channels.
 */

#include "test.h"

// MANUAL_REVIEW: Headers field was NA in the Meta TestPlan JSON.
// No Meta TestPlan headers were provided. The following headers are inferred
// from the test description and impacted registers. Verify and adjust as needed.
// #include <ethernet0/ethernet0_def.h>
// #include <ethernet0/ethernet0_offset.h>

/* Transfer count configuration */
// MANUAL_REVIEW: Default transfer count is 10, reduced to 6 for 10M speed.
// Adjust DEFAULT_TRANSFER_COUNT based on the target speed configuration.
#ifndef DEFAULT_TRANSFER_COUNT
#define DEFAULT_TRANSFER_COUNT 10U
#endif

/* Polling timeout for interrupt wait loop */
// MANUAL_REVIEW: Set POLL_TIMEOUT_COUNT to an appropriate value for the
// target platform to prevent infinite polling.
#ifndef POLL_TIMEOUT_COUNT
#define POLL_TIMEOUT_COUNT 1000000U
#endif

/* TX descriptor ring length (10 per the test steps) */
#ifndef TX_DESC_RING_LEN
#define TX_DESC_RING_LEN 10U
#endif

/*
 * Register configuration values
 * MANUAL_REVIEW: All register configuration values below are placeholders.
 * Replace with the correct values from the Ethernet0 IP programming guide
 * or the original DV testcase source.
 */

/* Step 4: MAC Address slot values */
#ifndef MAC_ADDR0_HIGH_VALUE
#define MAC_ADDR0_HIGH_VALUE 0x00000000U
#endif

#ifndef MAC_ADDR0_LOW_VALUE
#define MAC_ADDR0_LOW_VALUE 0x00000000U
#endif

#ifndef MAC_ADDR1_HIGH_VALUE
#define MAC_ADDR1_HIGH_VALUE 0x00000000U
#endif

#ifndef MAC_ADDR1_LOW_VALUE
#define MAC_ADDR1_LOW_VALUE 0x00000000U
#endif

#ifndef MAC_ADDR2_HIGH_VALUE
#define MAC_ADDR2_HIGH_VALUE 0x00000000U
#endif

#ifndef MAC_ADDR2_LOW_VALUE
#define MAC_ADDR2_LOW_VALUE 0x00000000U
#endif

#ifndef MAC_ADDR3_HIGH_VALUE
#define MAC_ADDR3_HIGH_VALUE 0x00000000U
#endif

#ifndef MAC_ADDR3_LOW_VALUE
#define MAC_ADDR3_LOW_VALUE 0x00000000U
#endif

/* Step 5: MAC Packet Filter - hash filtering and receive-all */
#ifndef MAC_PACKET_FILTER_VALUE
#define MAC_PACKET_FILTER_VALUE 0x00000000U
#endif

/* Step 6: MAC Configuration - enable TX, RX, speed, duplex */
#ifndef MAC_CFG_VALUE
#define MAC_CFG_VALUE 0x00000000U
#endif

/* Step 8: MAC Extended Configuration */
#ifndef MAC_EXT_CFG_VALUE
#define MAC_EXT_CFG_VALUE 0x00000000U
#endif

/* Step 9: MTL TX Queue Operation Mode values (4096-byte, store-and-forward) */
#ifndef MTL_TXQ0_OP_MODE_VALUE
#define MTL_TXQ0_OP_MODE_VALUE 0x00000000U
#endif

#ifndef MTL_TXQ1_OP_MODE_VALUE
#define MTL_TXQ1_OP_MODE_VALUE 0x00000000U
#endif

#ifndef MTL_TXQ2_OP_MODE_VALUE
#define MTL_TXQ2_OP_MODE_VALUE 0x00000000U
#endif

#ifndef MTL_TXQ3_OP_MODE_VALUE
#define MTL_TXQ3_OP_MODE_VALUE 0x00000000U
#endif

/* Step 10: MTL TX Quantum Weight values */
#ifndef MTL_TXQ0_QW_VALUE
#define MTL_TXQ0_QW_VALUE 0x00000000U
#endif

#ifndef MTL_TXQ1_QW_VALUE
#define MTL_TXQ1_QW_VALUE 0x00000000U
#endif

#ifndef MTL_TXQ2_QW_VALUE
#define MTL_TXQ2_QW_VALUE 0x00000000U
#endif

#ifndef MTL_TXQ3_QW_VALUE
#define MTL_TXQ3_QW_VALUE 0x00000000U
#endif

/* Step 11: MTL Queue Interrupt Control values */
#ifndef MTL_Q0_INT_CTRL_VALUE
#define MTL_Q0_INT_CTRL_VALUE 0x00000000U
#endif

#ifndef MTL_Q1_INT_CTRL_VALUE
#define MTL_Q1_INT_CTRL_VALUE 0x00000000U
#endif

#ifndef MTL_Q2_INT_CTRL_VALUE
#define MTL_Q2_INT_CTRL_VALUE 0x00000000U
#endif

#ifndef MTL_Q3_INT_CTRL_VALUE
#define MTL_Q3_INT_CTRL_VALUE 0x00000000U
#endif

/* Step 12: DMA System Bus Mode */
#ifndef DMA_SYSBUS_MODE_VALUE
#define DMA_SYSBUS_MODE_VALUE 0x00000000U
#endif

/* Step 15: DMA TX Descriptor List Address values */
#ifndef DMA_CH0_TXDESC_LIST_ADDR_VALUE
#define DMA_CH0_TXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

#ifndef DMA_CH1_TXDESC_LIST_ADDR_VALUE
#define DMA_CH1_TXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

#ifndef DMA_CH2_TXDESC_LIST_ADDR_VALUE
#define DMA_CH2_TXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

#ifndef DMA_CH3_TXDESC_LIST_ADDR_VALUE
#define DMA_CH3_TXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

/* Step 16: DMA TX Descriptor Tail Pointer values */
#ifndef DMA_CH0_TXDESC_TAIL_PTR_VALUE
#define DMA_CH0_TXDESC_TAIL_PTR_VALUE 0x00000000U
#endif

#ifndef DMA_CH1_TXDESC_TAIL_PTR_VALUE
#define DMA_CH1_TXDESC_TAIL_PTR_VALUE 0x00000000U
#endif

#ifndef DMA_CH2_TXDESC_TAIL_PTR_VALUE
#define DMA_CH2_TXDESC_TAIL_PTR_VALUE 0x00000000U
#endif

#ifndef DMA_CH3_TXDESC_TAIL_PTR_VALUE
#define DMA_CH3_TXDESC_TAIL_PTR_VALUE 0x00000000U
#endif

/* Step 17: DMA Channel Control values */
#ifndef DMA_CH0_CTRL_VALUE
#define DMA_CH0_CTRL_VALUE 0x00000000U
#endif

#ifndef DMA_CH1_CTRL_VALUE
#define DMA_CH1_CTRL_VALUE 0x00000000U
#endif

#ifndef DMA_CH2_CTRL_VALUE
#define DMA_CH2_CTRL_VALUE 0x00000000U
#endif

#ifndef DMA_CH3_CTRL_VALUE
#define DMA_CH3_CTRL_VALUE 0x00000000U
#endif

/* Step 18: DMA Interrupt Enable value */
#ifndef DMA_CHX_INT_ENABLE_VALUE
#define DMA_CHX_INT_ENABLE_VALUE 0x00000000U
#endif

/* Step 20: DMA CH0 TX Control with start TX bit */
#ifndef DMA_CH0_TX_CTRL_START_VALUE
#define DMA_CH0_TX_CTRL_START_VALUE 0x00000000U
#endif
