/*
 * ethernet0_tx_rx_multi_chnl_test.c
 *
 * PSV testcase: ethernet0_tx_rx_multi_chnl_test
 * Multi-channel TX/RX test for Ethernet0.
 *
 * Auto-generated PSV adaptation from DV-derived Meta testcase.
 /

#include "ethernet0_tx_rx_multi_chnl_test.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <test_common.h>
#include <ethernet0/ethernet0_def.h>
#include <ethernet0/ethernet0_offset.h>
#include <ethernet0/ethernet0_funcs.h>
#include <ethernet0/ethernet0_programming_sequence.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/
 * MANUAL_REVIEW: ATP Section 9.19 requires external Ethernet validation:
 * - External endpoint must be identified.
 * - Hardware topology must be defined (RJ45 cable, network switch).
 * - Communication path must be documented.
 * The DV source does not identify an external endpoint or document
 * the hardware topology for PSV. External loopback or switch-based
 * validation is required by the ATP but cannot be implemented without
 * explicit PSV topology information.
 /

/
 * MANUAL_REVIEW: ATP Section 9.19 requires end-to-end Ethernet validation:
 * - Validate TX frame content.
 * - Validate RX frame content.
 * - Validate payload integrity.
 * - Validate source and destination MAC addresses.
 * - Include external packet verification where applicable.
 * - Do not rely solely on TX/RX counters for PASS criteria.
 * The DV source only programs registers and writes descriptor tail
 * pointers. No TX/RX frame validation, payload integrity check, or
 * MAC address verification is present. These validations cannot be
 * invented without explicit expected values and verification APIs.
 /

/
 * MANUAL_REVIEW: ATP Section 9.19 requires duplex validation:
 * - Verify the configured duplex mode through readback/status checks.
 * - If ATP requires both modes, validate both independently.
 * The DV source configures duplex mode but performs no readback or
 * status verification of the configured mode. Readback validation
 * cannot be invented without explicit expected values and register
 * read specifications.
 /

#error "MANUAL_REVIEW: ATP requires external endpoint identification, end-to-end TX/RX frame and payload validation, MAC address verification, and duplex readback verification -- none present in DV source and cannot be invented"

/
 * MANUAL_REVIEW: DV step 3 write_reg(0xA0243ffc, enet_sel) is a VIP
 * sequencer handshake (DV infrastructure). This is NOT the exact approved
 * debug marker exception (write_reg(0xA0243ffc, 0xdeadbee0)) because the
 * write data differs. No explicit PSV replacement is supplied. The
 * operation is omitted as DV-only VIP synchronization per Section 9.E.
 * If this write is required on silicon, supply an explicit PSV equivalent.
 /

/
 * MANUAL_REVIEW: Original source declares 'extern int int_pend;'.
 * Per user rules, changed to a volatile definition. This changes linkage
 * from an external declaration to a file-scope definition. Verify that
 * no other translation unit defines this symbol, or restore extern linkage
 * if int_pend is defined elsewhere.
 /
volatile int int_pend;

/
 * MANUAL_REVIEW: enet_sel is used in write_reg((0xE68C205C + (enet_sel * 4)),
 * 0x1A08) but is not declared in this file. It is presumed to be a global
 * variable set by enet_intf_sel() and declared in one of the ethernet0
 * headers. Verify that enet_sel is provided by the included ethernet0 headers.
 */


int ethernet0_tx_rx_multi_chnl_test_init(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("ethernet0_tx_rx_multi_chnl_test init");

 return 0;
}


int ethernet0_tx_rx_multi_chnl_test_run(const TestsItem *cfg, TestOutput out)
{
 (void)cfg;

 if (out == 0) {
 LOGE("Output pointer is NULL");
 return -1;
 }

 out->status = 0;
 out->actual_len = 0;
 out->actual_pattern[0] = 0;

 LOGT("ethernet0_tx_rx_multi_chnl_test run start");

 / Step 2: Select Ethernet interface via preprocessor conditionals /
 /
 * MANUAL_REVIEW: Original DV source uses '#elif SEL_ENET1' etc. without
 * the defined() operator. This evaluates the macro value rather than
 * testing for macro definition, which may be a DV source defect.
 * Preserved as-is per functional preservation rules.
 /
#ifdef SEL_ENET0
 enet_intf_sel(0);
#elif SEL_ENET1
 enet_intf_sel(1);
#elif SEL_ENET2
 enet_intf_sel(2);
#elif SEL_ENET3
 enet_intf_sel(3);
#else
 enet_intf_sel(4);
#endif

 /
 * MANUAL_REVIEW: DV step 3 write_reg(0xA0243ffc, enet_sel) omitted.
 * VIP sequencer handshake with no PSV equivalent supplied.
 /
 LOGT("DV VIP sequencer write omitted (0xA0243ffc)");

 / Step 4: Configure MAC based on speed and duplex /
 /
 * MANUAL_REVIEW: Original DV source uses '#elif ETH_100M' without the
 * defined() operator. This evaluates the macro value rather than testing
 * for macro definition, which may be a DV source defect. Preserved as-is
 * per functional preservation rules.
 /
#ifdef ETH_10M
#ifdef HALF_DUPLEX
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x8003);
#else
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xA003);
#endif
 /
 * MANUAL_REVIEW: write_reg to address (0xE68C205C + (enet_sel * 4))
 * is not mapped to a named register per source remarks. Preserved
 * as-is; verify this override enable address is valid on silicon.
 /
 write_reg((0xE68C205C + (enet_sel * 4)), 0x1A08);
 / Original: wait_on(300) /
 for (volatile unsigned int d = 0U; d < 10000U; d++);
#elif ETH_100M
#ifdef HALF_DUPLEX
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xC003);
#else
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xE003);
#endif
 /
 * MANUAL_REVIEW: write_reg to address (0xE68C205C + (enet_sel * 4))
 * is not mapped to a named register per source remarks. Preserved
 * as-is; verify this override enable address is valid on silicon.
 /
 write_reg((0xE68C205C + (enet_sel * 4)), 0x1A08);
 / Original: wait_on(300) /
 for (volatile unsigned int d = 0U; d < 10000U; d++);
#else
 /
 * MANUAL_REVIEW: Neither ETH_10M nor ETH_100M is defined.
 * MAC configuration step is entirely skipped. The original DV
 * source has the same gap. Define ETH_10M or ETH_100M to select
 * the desired speed configuration.
 /
#error "MANUAL_REVIEW: Neither ETH_10M nor ETH_100M is defined -- MAC speed configuration is skipped"
#endif

 / Step 5: Clear MAC_EXT_CONFIGURATION /
 write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x0);

 / Step 6: Program MTL TX Queue Operation Modes for TXQ0-TXQ2 /
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE, 0x0003000a);
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE, 0x0003000a);
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE, 0x0003000a);

 / Quantum weights for TXQ1-TXQ3 /
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT, 0x00000005);
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT, 0x00000014);
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT, 0x00000014);

 / MTL Operation Mode /
 write_enet_reg(mizar_ETHERNET0_MTL_OPERATION_MODE, 0x00000000);

 / MTL RX Queue Operation Modes for RXQ0-RXQ3 /
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ0_OPERATION_MODE, 0x00f00033);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ1_OPERATION_MODE, 0x00f00033);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ2_OPERATION_MODE, 0x00f00033);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ3_OPERATION_MODE, 0x00f00033);

 / Step 7: MAC RXQ Control /
 write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL0, 0x00aa);
 write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL1, 0x000);

 / Step 8: Map RX Queues to DMA Channels /
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000083);

 / Step 9: Program MAC Address LOW registers for multi-channel routing /
 write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_LOW, 0x08090a0b);
 write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_LOW, 0x08090a0b);
 write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_LOW, 0x08090a0b);
 write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW, 0x08090a0b);

 / Step 10: Configure MAC Packet Filter /
 write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER, 0x00000408);

 / Step 11: Set DMA Mode /
 write_enet_reg(mizar_ETHERNET0_DMA_MODE, 0x00000000);

 / Step 12: Configure DMA CH0 Control /
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000);

 / Step 13: Program DMA TX Control for CH3-CH0 /
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100006);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100006);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100006);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100006);

 / Step 14: Program DMA RX Control for CH3-CH0 /
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101000);

 / Step 15: Set TX Descriptor List base addresses for CH3-CH0 /
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0xE6030000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0xE6020000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0xE6010000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0xE6000000);

 / Step 16: Set RX Descriptor List base addresses for CH3-CH0 /
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_LIST_ADDRESS, 0xE6070000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_LIST_ADDRESS, 0xE6060000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_LIST_ADDRESS, 0xE6050000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS, 0xE6040000);

 / Step 17: Program TX Descriptor Ring Lengths for CH1-CH3 /
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_RING_LENGTH, 0x9);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_RING_LENGTH, 0x9);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_RING_LENGTH, 0x9);

 / Step 18: Configure DMA CH0 RX Control2 and Interrupt Enable /
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL2, 0x9);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE, 0xf0df);

 / Step 19: Write TX Descriptor Tail Pointers to initiate DMA transfers /
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0xE6039EB4);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0xE6029EB4);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER, 0xE6019EB4);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER, 0xE6009EB4);

 LOGT("All DMA tail pointers written, transfers initiated");

 /
 * MANUAL_REVIEW: No validation/acceptance criteria are implemented.
 * ATP requires: TX/RX frame validation, payload integrity,
 * MAC address verification, duplex mode readback, and external
 * packet verification. The DV source contains no pass/fail checks.
 * Cannot invent validation reads, expected values, or APIs.
 */

 LOGT("ethernet0_tx_rx_multi_chnl_test run complete: status=%d",
 out->status);

 return out->status;
}


int ethernet0_tx_rx_multi_chnl_test_teardown(const TestsItem cfg)
{
 (void)cfg;

 LOGT("ethernet0_tx_rx_multi_chnl_test teardown");

 return 0;
}
