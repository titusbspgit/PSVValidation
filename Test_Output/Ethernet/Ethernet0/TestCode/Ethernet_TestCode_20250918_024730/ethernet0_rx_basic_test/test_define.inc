// Author - AI Force 2.3. 18-Sep-2025 08:17 IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: ethernet0_rx_basic_test
 * Purpose:   Verifies basic Ethernet RX packet reception on Ethernet0 using
 *            interrupt-driven DMA transfers across four channels.
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

/* Speed configuration macros */
// MANUAL_REVIEW: ETH_10M value was not provided in the Meta TestPlan JSON.
// Define ETH_10M with the correct speed configuration value.
#ifndef ETH_10M
#define ETH_10M 0U
#endif

// MANUAL_REVIEW: ETH_100M value was not provided in the Meta TestPlan JSON.
// Define ETH_100M with the correct speed configuration value.
#ifndef ETH_100M
#define ETH_100M 1U
#endif

/* Duplex mode macros */
// MANUAL_REVIEW: HALF_DUPLEX value was not provided in the Meta TestPlan JSON.
// Define HALF_DUPLEX with the correct duplex mode value.
#ifndef HALF_DUPLEX
#define HALF_DUPLEX 0U
#endif

/* Ethernet instance selection macros */
// MANUAL_REVIEW: SEL_ENET0-3 values were not provided in the Meta TestPlan JSON.
// Define with the correct instance selection values.
#ifndef SEL_ENET0
#define SEL_ENET0 0U
#endif

#ifndef SEL_ENET1
#define SEL_ENET1 1U
#endif

#ifndef SEL_ENET2
#define SEL_ENET2 2U
#endif

#ifndef SEL_ENET3
#define SEL_ENET3 3U
#endif

/* Debug display macro */
// MANUAL_REVIEW: DEBUG_DISPLAY is referenced in the source. Define as needed.
#ifndef DEBUG_DISPLAY
#define DEBUG_DISPLAY 0U
#endif

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

/* DMA descriptor ring length */
#ifndef DMA_DESC_RING_LEN_32
#define DMA_DESC_RING_LEN_32 32U
#endif

/*
 * Register configuration values
 * MANUAL_REVIEW: All register configuration values below are placeholders.
 * Replace with the correct values from the Ethernet0 IP programming guide
 * or the original DV testcase source.
 */

/* Step 3: MAC Configuration - enable TX, RX, speed, duplex */
#ifndef MAC_CFG_VALUE
#define MAC_CFG_VALUE 0x00000000U
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

#ifndef MAC_RXQ_CTRL4_VALUE
#define MAC_RXQ_CTRL4_VALUE 0x00000000U
#endif

/* Step 5: MAC VLAN Tag Control */
#ifndef MAC_VLAN_TAG_CTRL_VALUE
#define MAC_VLAN_TAG_CTRL_VALUE 0x00000000U
#endif

/* Step 6: MAC Packet Filter */
#ifndef MAC_PACKET_FILTER_VALUE
#define MAC_PACKET_FILTER_VALUE 0x00000000U
#endif

/* Step 7: MAC Address slot values */
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

/* Step 11: MTL RX Queue Operation Mode values (4096-byte, store-and-forward) */
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

/* Step 12: MTL RX Queue-to-DMA Channel Mapping */
#ifndef MTL_RXQ_DMA_MAP0_VALUE
#define MTL_RXQ_DMA_MAP0_VALUE 0x00000000U
#endif

/* Step 13: MTL Queue Interrupt Control values */
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

/* Step 14: MTL RX Queue Control values */
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

/* Step 14: MTL Operation Mode */
#ifndef MTL_OP_MODE_VALUE
#define MTL_OP_MODE_VALUE 0x00000000U
#endif

/* Step 15: DMA TX Control value (16-beat burst, channel weight) */
#ifndef DMA_CHX_TX_CTRL_VALUE
#define DMA_CHX_TX_CTRL_VALUE 0x00000000U
#endif

/* Step 16: DMA TX Descriptor List Address values */
#ifndef DMA_CH1_TXDESC_LIST_ADDR_VALUE
#define DMA_CH1_TXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

#ifndef DMA_CH2_TXDESC_LIST_ADDR_VALUE
#define DMA_CH2_TXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

#ifndef DMA_CH3_TXDESC_LIST_ADDR_VALUE
#define DMA_CH3_TXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

/* Step 17: DMA RX Control value (16-beat burst, RX buffer size) */
#ifndef DMA_CHX_RX_CTRL_VALUE
#define DMA_CHX_RX_CTRL_VALUE 0x00000000U
#endif

/* Step 18: DMA System Bus Mode */
#ifndef DMA_SYSBUS_MODE_VALUE
#define DMA_SYSBUS_MODE_VALUE 0x00000000U
#endif

/* Step 19: DMA RX Descriptor List Address values */
#ifndef DMA_CH1_RXDESC_LIST_ADDR_VALUE
#define DMA_CH1_RXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

#ifndef DMA_CH2_RXDESC_LIST_ADDR_VALUE
#define DMA_CH2_RXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

#ifndef DMA_CH3_RXDESC_LIST_ADDR_VALUE
#define DMA_CH3_RXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

/* Step 21: DMA CH0 TX Control */
#ifndef DMA_CH0_TX_CTRL_VALUE
#define DMA_CH0_TX_CTRL_VALUE 0x00000000U
#endif

/* Step 21: DMA CH0 TX Descriptor List Address */
#ifndef DMA_CH0_TXDESC_LIST_ADDR_VALUE
#define DMA_CH0_TXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

/* Step 22: DMA Interrupt Enable value */
#ifndef DMA_CHX_INT_ENABLE_VALUE
#define DMA_CHX_INT_ENABLE_VALUE 0x00000000U
#endif

/* Step 23: DMA CH0 RX Control */
#ifndef DMA_CH0_RX_CTRL_VALUE
#define DMA_CH0_RX_CTRL_VALUE 0x00000000U
#endif

/* Step 23: DMA CH0 RX Descriptor List Address */
#ifndef DMA_CH0_RXDESC_LIST_ADDR_VALUE
#define DMA_CH0_RXDESC_LIST_ADDR_VALUE 0x00000000U
#endif

/* Step 24: MAC Interrupt Enable */
#ifndef MAC_INT_ENABLE_VALUE
#define MAC_INT_ENABLE_VALUE 0x00000000U
#endif

/* Step 25: DMA Mode */
#ifndef DMA_MODE_VALUE
#define DMA_MODE_VALUE 0x00000000U
#endif

/* Step 26: DMA Channel Control values */
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

/* Step 26: DMA CH0 RX Descriptor Tail Pointer */
#ifndef DMA_CH0_RXDESC_TAIL_PTR_VALUE
#define DMA_CH0_RXDESC_TAIL_PTR_VALUE 0x00000000U
#endif
