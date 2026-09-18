// Author - AI Force 2.3. 18-Sep-2025 08:17 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: ethernet0_tx_rx_multi_chnl_test
 * Purpose:   Verifies multi-channel TX and RX Ethernet packet transfer on
 *            Ethernet0 using all four DMA channels in sequential phases.
 */

#include "test.h"

// MANUAL_REVIEW: Headers field was NA in the Meta TestPlan JSON.
// No Meta TestPlan headers were provided. The following headers are inferred
// from the test description and impacted registers. Verify and adjust as needed.
// #include <ethernet0/ethernet0_def.h>
// #include <ethernet0/ethernet0_offset.h>

/* Phase target packet counts */
#ifndef PHASE1_TARGET
#define PHASE1_TARGET 10U
#endif

#ifndef PHASE2_TARGET
#define PHASE2_TARGET 20U
#endif

#ifndef PHASE3_TARGET
#define PHASE3_TARGET 30U
#endif

#ifndef PHASE4_TARGET
#define PHASE4_TARGET 40U
#endif

/* Polling max iterations per phase */
// MANUAL_REVIEW: Set POLL_MAX_ITERATIONS to an appropriate value for the
// target platform. Original source uses 20 iterations per phase.
#ifndef POLL_MAX_ITERATIONS
#define POLL_MAX_ITERATIONS 20U
#endif

/* TX descriptor ring length (10 per the test steps) */
#ifndef TX_DESC_RING_LEN
#define TX_DESC_RING_LEN 10U
#endif

/* DMA RX ring length */
// MANUAL_REVIEW: Set DMA_RX_RING_LEN_VALUE to the correct RX ring length.
#ifndef DMA_RX_RING_LEN_VALUE
#define DMA_RX_RING_LEN_VALUE 0x00000000U
#endif

/*
 * Register configuration values
 * MANUAL_REVIEW: All register configuration values below are placeholders.
 * Replace with the correct values from the Ethernet0 IP programming guide
 * or the original DV testcase source.
 */

/* Step 3: MAC Configuration */
#ifndef MAC_CFG_VALUE
#define MAC_CFG_VALUE 0x00000000U
#endif

/* Step 4: MAC Extended Configuration */
#ifndef MAC_EXT_CFG_VALUE
#define MAC_EXT_CFG_VALUE 0x00000000U
#endif

/* Step 4: MAC RX Queue Control values */
#ifndef MAC_RXQ_CTRL0_VALUE
#define MAC_RXQ_CTRL0_VALUE 0x00000000U
#endif

#ifndef MAC_RXQ_CTRL1_VALUE
#define MAC_RXQ_CTRL1_VALUE 0x00000000U
#endif

#ifndef MAC_RXQ_CTRL2_VALUE
#define MAC_RXQ_CTRL2_VALUE 0x00000000U
#endif

/* Step 4: MAC VLAN Tag Control */
#ifndef MAC_VLAN_TAG_CTRL_VALUE
#define MAC_VLAN_TAG_CTRL_VALUE 0x00000000U
#endif

/* Step 4: MAC Packet Filter */
#ifndef MAC_PACKET_FILTER_VALUE
#define MAC_PACKET_FILTER_VALUE 0x00000000U
#endif

/* Step 5: MMC IPC RX Interrupt Mask */
#ifndef MMC_IPC_RX_INT_MASK_VALUE
#define MMC_IPC_RX_INT_MASK_VALUE 0x00000000U
#endif

/* Step 6: MAC Address slot values */
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

/* Step 7: MTL TX Queue Operation Mode values (1KB, store-and-forward) */
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

/* Step 8: MTL TX Quantum Weight values (Q3/Q2=20, Q1/Q0=5) */
#ifndef MTL_TXQ0_QW_VALUE
#define MTL_TXQ0_QW_VALUE 0x00000005U
#endif

#ifndef MTL_TXQ1_QW_VALUE
#define MTL_TXQ1_QW_VALUE 0x00000005U
#endif

#ifndef MTL_TXQ2_QW_VALUE
#define MTL_TXQ2_QW_VALUE 0x00000014U
#endif

#ifndef MTL_TXQ3_QW_VALUE
#define MTL_TXQ3_QW_VALUE 0x00000014U
#endif

/* Step 9: MTL Operation Mode */
#ifndef MTL_OP_MODE_VALUE
#define MTL_OP_MODE_VALUE 0x00000000U
#endif

/* Step 10: MTL RX Queue Operation Mode values */
#ifndef MTL_RXQ0_OP_MODE_VALUE
#define MTL_RXQ0_OP_MODE_VALUE 0x00000000U
#endif

#ifndef MTL_RXQ1_OP_MODE_VALUE
#define MTL_RXQ1_OP_MODE_VALUE 0x00000000U
#endif

#ifndef MTL_RXQ2_OP_MODE_VALUE
#define MTL_RXQ2_OP_MODE_VALUE 0x00000000U
#endif

#ifndef MTL_RXQ3_OP_MODE_VALUE
#define MTL_RXQ3_OP_MODE_VALUE 0x00000000U
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

/* Step 12: MTL RX Queue-to-DMA Channel Mapping (initial) */
#ifndef MTL_RXQ_DMA_MAP0_VALUE
#define MTL_RXQ_DMA_MAP0_VALUE 0x00000000U
#endif

/* Phase-specific RX queue mapping values */
// MANUAL_REVIEW: Set the correct mapping values to direct all RX traffic
// to the active channel for each phase.
#ifndef RXQ_MAP_CH3_VALUE
#define RXQ_MAP_CH3_VALUE 0x00000000U
#endif

#ifndef RXQ_MAP_CH2_VALUE
#define RXQ_MAP_CH2_VALUE 0x00000000U
#endif

#ifndef RXQ_MAP_CH1_VALUE
#define RXQ_MAP_CH1_VALUE 0x00000000U
#endif

#ifndef RXQ_MAP_CH0_VALUE
#define RXQ_MAP_CH0_VALUE 0x00000000U
#endif

/* Step 13: MTL RX Queue Control values */
#ifndef MTL_RXQ0_CTRL_VALUE
#define MTL_RXQ0_CTRL_VALUE 0x00000000U
#endif

#ifndef MTL_RXQ1_CTRL_VALUE
#define MTL_RXQ1_CTRL_VALUE 0x00000000U
#endif

#ifndef MTL_RXQ2_CTRL_VALUE
#define MTL_RXQ2_CTRL_VALUE 0x00000000U
#endif

#ifndef MTL_RXQ3_CTRL_VALUE
#define MTL_RXQ3_CTRL_VALUE 0x00000000U
#endif

/* Step 14: DMA TX Control value (16-beat burst) */
#ifndef DMA_CHX_TX_CTRL_VALUE
#define DMA_CHX_TX_CTRL_VALUE 0x00000000U
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

/* Step 18: DMA System Bus Mode */
#ifndef DMA_SYSBUS_MODE_VALUE
#define DMA_SYSBUS_MODE_VALUE 0x00000000U
#endif

/* Step 19: DMA RX Control value (16-beat burst, RX buffer size) */
#ifndef DMA_CHX_RX_CTRL_VALUE
#define DMA_CHX_RX_CTRL_VALUE 0x00000000U
#endif

/* Step 21: DMA RX Descriptor List Address values */
#ifndef DMA_CH0_RXDESC_LIST_ADDR_VALUE
#define DMA_CH0_RXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

#ifndef DMA_CH1_RXDESC_LIST_ADDR_VALUE
#define DMA_CH1_RXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

#ifndef DMA_CH2_RXDESC_LIST_ADDR_VALUE
#define DMA_CH2_RXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

#ifndef DMA_CH3_RXDESC_LIST_ADDR_VALUE
#define DMA_CH3_RXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

/* Step 25: DMA Interrupt Enable value */
#ifndef DMA_CHX_INT_ENABLE_VALUE
#define DMA_CHX_INT_ENABLE_VALUE 0x00000000U
#endif

/* TX Descriptor Tail Pointer values */
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

/* RX Descriptor Tail Pointer values */
#ifndef DMA_CH0_RXDESC_TAIL_PTR_VALUE
#define DMA_CH0_RXDESC_TAIL_PTR_VALUE 0x00000000U
#endif

#ifndef DMA_CH1_RXDESC_TAIL_PTR_VALUE
#define DMA_CH1_RXDESC_TAIL_PTR_VALUE 0x00000000U
#endif

#ifndef DMA_CH2_RXDESC_TAIL_PTR_VALUE
#define DMA_CH2_RXDESC_TAIL_PTR_VALUE 0x00000000U
#endif

#ifndef DMA_CH3_RXDESC_TAIL_PTR_VALUE
#define DMA_CH3_RXDESC_TAIL_PTR_VALUE 0x00000000U
#endif

/* DMA Mode */
#ifndef DMA_MODE_VALUE
#define DMA_MODE_VALUE 0x00000000U
#endif

/* DMA TX/RX Start values (with start bit set) */
#ifndef DMA_CHX_TX_START_VALUE
#define DMA_CHX_TX_START_VALUE 0x00000000U
#endif

#ifndef DMA_CHX_RX_START_VALUE
#define DMA_CHX_RX_START_VALUE 0x00000000U
#endif
