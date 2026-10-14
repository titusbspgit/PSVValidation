// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_tx_basic_test.h"
#include "test_define.inc"

// Meta Step 9: Global declarations
static int data_rd;
static int data_wr;
static int def_fail_cnt = 0;
static int wr_fail_cnt = 0;
static unsigned int trns_count = 10;

// MANUAL_REVIEW: extern int int_pend is a DV-derived interrupt pending flag.
// In PSV, this must be provided by the interrupt framework or a shared
// volatile variable updated by the ISR. Ensure int_pend is declared and
// updated by the actual PSV interrupt handler.
extern int int_pend;

// MANUAL_REVIEW: enet_sel is used in the DV source for VIP sequencer trigger.
// In PSV, this variable must be defined or provided by the Ethernet
// framework/driver. Declaring here to preserve compilation.
static int enet_sel = 4;

// =========================================================================
// ATP COMPLIANCE FLAGS (Section 9.19 Ethernet)
// =========================================================================
// MANUAL_REVIEW: [ATP-ETH-001] External Ethernet Validation Gap:
// ATP requires: External PHY Loopback topology (MIZAR SoC -> ETHERNET PHY -> SWITCH).
// Hardware: RJ45 Ethernet cable, network switch, UART USB cable, JTAG debugger, oscilloscope.
// This test does NOT identify an external endpoint, define hardware topology,
// or document the communication path. Ensure PSV test setup includes the
// required external PHY loopback hardware before execution.
//
// MANUAL_REVIEW: [ATP-ETH-002] End-to-End Ethernet Validation Gap:
// ATP requires: Validate TX frame, RX frame, payload integrity, source/destination
// MAC addresses, and external packet verification.
// This test is TX-only. It does NOT validate RX frames, payload integrity,
// or MAC addresses in received frames. It relies on interrupt completion
// (int_pend) as the sole PASS criteria, not end-to-end data verification.
// To satisfy ATP, add RX path validation, payload integrity checks, and
// MAC address verification (e.g., via Wireshark capture or loopback RX).
//
// MANUAL_REVIEW: [ATP-ETH-003] Duplex Validation Gap:
// ATP requires: Configure duplex mode, use distinct configs for Half/Full Duplex,
// verify configured mode through readback/status checks.
// This test configures duplex via MAC_CONFIGURATION register but does NOT
// perform a readback to verify the configured duplex mode was applied.
// Add a read_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION) after writing
// and verify the speed/duplex bits match the intended configuration.
// =========================================================================


/*
 * Function: Default_IRQHandler
 * Description: Default IRQ handler for Ethernet0 TX interrupt completion.
 * Clears the int_pend flag when a transfer interrupt is received.
 * Parameters:
 * None
 * Returns:
 * void
 /
// MANUAL_REVIEW: Default_IRQHandler is a DV-derived ISR. In PSV, this must
// be registered with the actual interrupt controller. The ISR body must
// clear int_pend and handle DMA CH0 TX completion interrupt status.
// Provide the actual PSV ISR implementation.
void Default_IRQHandler(void)
{
 // MANUAL_REVIEW: Implement PSV interrupt handler body.
 // Expected behavior: clear int_pend flag upon TX completion interrupt.
 int_pend = 0;
}

/
 * Function: ethernet0_tx_basic_test_init
 * Description: Initialize Ethernet0 TX basic test. Select Ethernet interface,
 * configure MAC, MTL, and DMA layers for TX operation.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * FV/template-compatible status.
 */
int ethernet0_tx_basic_test_init(const TestsItem cfg)
{
 (void)cfg;

 def_fail_cnt = 0;
 wr_fail_cnt = 0;
 trns_count = 10;
 data_rd = 0;
 data_wr = 0;

 LOGT("ethernet0_tx_basic_test_init: Initializing Ethernet0 TX basic test");

 // Meta Step 12: Ethernet interface selection
#ifdef SEL_ENET0
 enet_intf_sel(0);
 enet_sel = 0;
#elif defined(SEL_ENET1)
 enet_intf_sel(1);
 enet_sel = 1;
#elif defined(SEL_ENET2)
 enet_intf_sel(2);
 enet_sel = 2;
#elif defined(SEL_ENET3)
 enet_intf_sel(3);
 enet_sel = 3;
#else
 enet_intf_sel(4);
 enet_sel = 4;
#endif

 LOGT("ethernet0_tx_basic_test_init: enet_intf_sel completed, enet_sel=%d", enet_sel);

 // Meta Step 13: If ETH_10M mode, set trns_count to 6
#ifdef ETH_10M
 trns_count = 6;
#endif

 LOGT("ethernet0_tx_basic_test_init: trns_count=%u", trns_count);

 // Meta Step 14: MAC Configuration - speed and duplex settings
#ifdef ETH_10M
#ifdef HALF_DUPLEX
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x8003);
 LOGT("MAC_CONFIGURATION: 0x8003 (10M, Half-Duplex)");
#else
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xA003);
 LOGT("MAC_CONFIGURATION: 0xA003 (10M, Full-Duplex)");
#endif
#elif defined(ETH_100M)
#ifdef HALF_DUPLEX
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xC003);
 LOGT("MAC_CONFIGURATION: 0xC003 (100M, Half-Duplex)");
#else
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xE003);
 LOGT("MAC_CONFIGURATION: 0xE003 (100M, Full-Duplex)");
#endif
#else
#ifdef HALF_DUPLEX
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x0003);
 LOGT("MAC_CONFIGURATION: 0x0003 (1G, Half-Duplex)");
#else
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x2003);
 LOGT("MAC_CONFIGURATION: 0x2003 (1G, Full-Duplex)");
#endif
#endif

 // Meta Step 15: MAC_EXT_CONFIGURATION
 write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x0);
 LOGT("MAC_EXT_CONFIGURATION: 0x0");

 // Meta Step 16: MAC_ADDRESS0_HIGH
 write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH, 0x80000607);
 LOGT("MAC_ADDRESS0_HIGH: 0x80000607");

 // Meta Step 17: MAC_ADDRESS0_LOW
 write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW, 0x08090a0b);
 LOGT("MAC_ADDRESS0_LOW: 0x08090a0b");

 // Meta Step 18: DMA_CH3_CONTROL
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_CONTROL, 0x0);
 LOGT("DMA_CH3_CONTROL: 0x0");

 // Meta Step 19: DMA_CH2_CONTROL
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_CONTROL, 0x0);
 LOGT("DMA_CH2_CONTROL: 0x0");

 // Meta Step 20: DMA_CH1_CONTROL
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_CONTROL, 0x0);
 LOGT("DMA_CH1_CONTROL: 0x0");

 // Meta Step 21: DMA_CH0_CONTROL
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000);
 LOGT("DMA_CH0_CONTROL: 0x00000000");

 // Meta Step 22: MTL_TXQ0_OPERATION_MODE - 4096 Bytes TX QUEUE Size, STR&FWD Enabled, TX Q0 Enabled
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE, 0x000f000a);
 LOGT("MTL_TXQ0_OPERATION_MODE: 0x000f000a (4096B TX Queue, Store&Forward, Q0 Enabled)");

 // Meta Step 23: DMA_CH3_TXDESC_LIST_ADDRESS
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0x00300000);
 LOGT("DMA_CH3_TXDESC_LIST_ADDRESS: 0x00300000");

 // Meta Step 24: DMA_CH2_TXDESC_LIST_ADDRESS
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0x00200000);
 LOGT("DMA_CH2_TXDESC_LIST_ADDRESS: 0x00200000");

 // Meta Step 25: DMA_CH1_TXDESC_LIST_ADDRESS
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0x00100000);
 LOGT("DMA_CH1_TXDESC_LIST_ADDRESS: 0x00100000");

 // Meta Step 26: DMA_CH0_TXDESC_LIST_ADDRESS
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0xE6000000);
 LOGT("DMA_CH0_TXDESC_LIST_ADDRESS: 0xE6000000");

 // Meta Step 27: DMA_CH3_TXDESC_TAIL_POINTER
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0x00309730);
 LOGT("DMA_CH3_TXDESC_TAIL_POINTER: 0x00309730");

 // Meta Step 28: DMA_CH2_TXDESC_TAIL_POINTER
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0x00209730);
 LOGT("DMA_CH2_TXDESC_TAIL_POINTER: 0x00209730");

 // Meta Step 29: DMA_CH1_TXDESC_TAIL_POINTER
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER, 0x00109730);
 LOGT("DMA_CH1_TXDESC_TAIL_POINTER: 0x00109730");

 // Meta Step 30: DMA_CH0_TXDESC_TAIL_POINTER
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER, 0xE6009EB4);
 LOGT("DMA_CH0_TXDESC_TAIL_POINTER: 0xE6009EB4");

 LOGT("ethernet0_tx_basic_test_init: Initialization complete");

 return 0;
}

/
 * Function: ethernet0_tx_basic_test_run
 * Description: Execute Ethernet0 basic TX test. Start TX on DMA Channel 0,
 * trigger VIP sequencer, and poll for transfer completion interrupts.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * out - pointer to TestOutput for result reporting
 * Returns:
 * FV/template-compatible status.
 */
int ethernet0_tx_basic_test_run(const TestsItem *cfg, TestOutput out)
{
 unsigned int i;

 (void)cfg;

 if (out == 0) {
 LOGE("ethernet0_tx_basic_test_run: output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("ethernet0_tx_basic_test_run: Starting TX on DMA Channel 0");

 // Meta Step 31: DMA_CH3_TX_CONTROL - commented out in source
 //write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100007); //START TX (commented out)

 // Meta Step 32: DMA_CH2_TX_CONTROL - commented out in source
 //write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100007); //START TX (commented out)

 // Meta Step 33: DMA_CH1_TX_CONTROL - commented out in source
 //write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100007); //START TX (commented out)

 // Meta Step 34: DMA_CH0_TX_CONTROL - START TX
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100007);
 LOGT("DMA_CH0_TX_CONTROL: 0x00100007 (START TX)");

 // Meta Step 35: write_reg(0xA0243ffc, enet_sel) - DV VIP sequencer trigger
 // MANUAL_REVIEW: This write_reg(0xA0243ffc, enet_sel) is a DV VIP sequencer
 // start trigger. Address 0xA0243ffc is DV/testbench infrastructure.
 // In PSV, the external traffic generator or PHY loopback must be started
 // through the actual hardware mechanism. Preserving as logging.
 LOGD("DV VIP sequencer trigger: write_reg(0xA0243ffc, enet_sel=%d) - not applicable in PSV", enet_sel);

 // Meta Step 36: write_reg(0xA0243ff8, 0xdeadbeef) - DV VIP sequencer trigger
 // MANUAL_REVIEW: This write_reg(0xA0243ff8, 0xdeadbeef) is a DV VIP sequencer
 // start trigger. Address 0xA0243ff8 is DV/testbench infrastructure.
 // In PSV, this is not applicable. Preserving as logging.
 LOGD("DV VIP sequencer trigger: write_reg(0xA0243ff8, 0xdeadbeef) - not applicable in PSV");

 // Meta Step 37: Poll for transfer completion interrupts
 // for(i = 0; i < trns_count; i++) { while(int_pend) { wait_on(10); } }
 // MANUAL_REVIEW: int_pend polling relies on Default_IRQHandler clearing int_pend.
 // Ensure the PSV interrupt framework is configured to route DMA CH0 TX
 // completion interrupts to Default_IRQHandler before this polling loop.
 LOGT("ethernet0_tx_basic_test_run: Polling for %u transfer completion interrupts", trns_count);

 for (i = 0; i < trns_count; i++) {
 // MANUAL_REVIEW: Added timeout protection for PSV. Original DV code
 // uses unbounded while(int_pend) with wait_on(10). In PSV, a timeout
 // prevents infinite hang if interrupt never arrives.
 volatile unsigned int timeout = 100000;
 while (int_pend) {
#ifdef DEBUG_DISPLAY
 LOGT("---->%u trnsf intrpt", i);
#endif
 // PSV adaptation of wait_on(10): deterministic busy-wait loop
 for (volatile int d = 0; d < 10000; d++);
 timeout--;
 if (timeout == 0) {
 LOGE("ethernet0_tx_basic_test_run: Timeout waiting for transfer %u interrupt", i);
 wr_fail_cnt++;
 break;
 }
 }
 LOGT("ethernet0_tx_basic_test_run: Transfer %u interrupt received", i);
 }

 LOGT("ethernet0_tx_basic_test_run: TX polling complete");

 // Meta Step 38: Pass/fail determination based on def_fail_cnt and wr_fail_cnt
 if ((def_fail_cnt == 0) && (wr_fail_cnt == 0)) {
 out->status = 0;
 LOGT("ethernet0_tx_basic_test_run: PASS - def_fail_cnt=%d wr_fail_cnt=%d",
 def_fail_cnt, wr_fail_cnt);
 } else {
 out->status = -1;
 LOGE("ethernet0_tx_basic_test_run: FAIL - def_fail_cnt=%d wr_fail_cnt=%d",
 def_fail_cnt, wr_fail_cnt);
 }

 return out->status;
}

/
 * Function: ethernet0_tx_basic_test_teardown
 * Description: Final cleanup and status reporting for Ethernet0 TX basic test.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * FV/template-compatible status.
 */
int ethernet0_tx_basic_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("ethernet0_tx_basic_test_teardown: def_fail_cnt=%d wr_fail_cnt=%d",
 def_fail_cnt, wr_fail_cnt);

 return ((def_fail_cnt == 0) && (wr_fail_cnt == 0)) ? 0 : -1;
}
