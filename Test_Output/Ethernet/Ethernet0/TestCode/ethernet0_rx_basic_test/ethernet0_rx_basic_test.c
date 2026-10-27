// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_rx_basic_test.h"
#include "test_define.inc"

// MANUAL_REVIEW: Runtime condition 1 (External Ethernet Validation) - ATP requires external Ethernet
// endpoint (RJ45 cable, network switch, traffic generator). DV uses VIP-driven packet stimulus.
// PSV must configure external Ethernet endpoint and validate communication path establishment.
// External endpoint configuration and topology initialization must be implemented here.

// MANUAL_REVIEW: Runtime condition 2 (End-to-End Ethernet Validation) - ATP requires actual Ethernet
// traffic validation including TX/RX frame verification, payload integrity, MAC address correctness,
// frame length, and packet content. DV source has no explicit packet content validation (unconditional
// finish(0)). PSV must add end-to-end packet verification for PASS criteria.

// MANUAL_REVIEW: Runtime condition 3 (Duplex Validation) - ATP requires half-duplex and full-duplex
// mode validation. DV uses #ifdef HALF_DUPLEX conditional compilation. PSV must verify programmed
// duplex mode through MAC_CONFIGURATION status readback after configuration.

// MANUAL_REVIEW: Runtime condition 5 (RX Descriptor Ring Maintenance) - DV source does not show
// explicit RX descriptor replenishment after packet reception in visible excerpts. PSV must preserve
// RX descriptor ring management and re-arm descriptors as required for continuous packet processing.

// MANUAL_REVIEW: Runtime condition 8 (Packet Stimulus Preservation) - DV uses VIP sequencer triggered
// by write_reg(0xA0243ffc,enet_sel) and write_reg(0xA0243ff8,0xdeadbeef). PSV must generate equivalent
// external packet stimulus (e.g., traffic generator on network switch or loopback).

// MANUAL_REVIEW: Runtime condition 9 (Hardware Counter Verification) - DV source does not show
// TX/RX packet counter reads or MAC/DMA statistics validation in visible excerpts. PSV must add
// hardware counter verification to supplement packet verification per ATP requirements.

// MANUAL_REVIEW: Runtime condition 14 (Variable Type Requirements) - Runtime requires unsigned int
// for all variables. Meta DV source uses signed int for data_rd, data_wr, def_fail_cnt, wr_fail_cnt,
// wdata, rdata, i, address, read. Preserving Meta source types to maintain DV functional equivalence.
// Signed behavior is used in DV for int_pend comparison and loop control.

// File-scope variables from DV source
int data_rd;
int data_wr;
int def_fail_cnt = 0;
int wr_fail_cnt = 0;
unsigned int trns_count = 10;

// MANUAL_REVIEW: int_pend is declared extern in DV source. It must be defined in the PSV
// environment (e.g., in a shared interrupt support module). Defining here for build correctness.
volatile int int_pend = 1;

// MANUAL_REVIEW: enet_select, enet_sel, and power are used in DV source but their declarations
// are not visible in retrieved excerpts. They may be declared in ethernet0_funcs.h or
// ethernet0_programming_sequence.h. If not provided by those headers, define them here.
// Declaring here for build correctness with MANUAL_REVIEW.
// extern int enet_select;
// extern int enet_sel;
// extern int power;

// Tracking variable for interrupt-based validation
static unsigned int irq_received_count = 0;
static unsigned int irq_errors = 0;


/*
 * Function: Default_IRQHandler
 * Description: Ethernet interrupt handler - reads DMA CH0-CH3 RX descriptor and buffer addresses,
 * reads and clears DMA interrupt status, writes power register, clears GIC IRQ
 * Parameters:
 * none
 * Returns:
 * void
 /
void Default_IRQHandler(void)
{
 int rdata, wdata;
 unsigned int irq_no;
 int_pend = 0;
 irq_no = 42 + enet_select;
 enet_sel = enet_select;

 LOGT("IN THE HANDLER %u for eth%d", irq_no, enet_select);

 // Read DMA CH0-CH3 current application RX descriptor and RX buffer addresses
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXBUFFER);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_RXDESC);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_RXBUFFER);

 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXDESC);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXBUFFER);

 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_RXDESC);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_RXBUFFER);

 // Read DMA interrupt status (first read)
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);

 LOGT("ETH DMA INT STS %x", rdata);

 // Clear DMA channel status by writing 0xffffffff to each channel
 wdata = 0xffffffff;
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_STATUS, wdata);
 wdata = 0xffffffff;
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_STATUS, wdata);
 wdata = 0xffffffff;
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_STATUS, wdata);
 wdata = 0xffffffff;
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_STATUS, wdata);

 // Re-read DMA interrupt status after clearing (second read)
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);

 LOGT("ETH DMA INT STS %x", rdata);

 // Write power register
 write_reg(0XE68C2050, power);

 // Clear GIC IRQ
 GIC_ClearIRQ(irq_no);

 LOGT("Clr IRQ");

 irq_received_count++;
}


/
 * Function: ethernet0_rx_basic_test_init
 * Description: Initializes GIC, selects Ethernet interface and speed/duplex mode,
 * programs MAC configuration registers
 * Parameters:
 * cfg - pointer to test configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int ethernet0_rx_basic_test_init(const TestsItem cfg)
{
 (void)cfg;

 int_pend = 1;
 def_fail_cnt = 0;
 wr_fail_cnt = 0;
 irq_received_count = 0;
 irq_errors = 0;

 int wdata, rdata;
 int i = 0;
 int address, read;

 LOGT("Ethernet0 RX basic test init - GIC and interface setup");

 // GIC initialization
 GIC_Set();
 GIC_EnableAllIRQ();

 // Speed selection via preprocessor conditionals
#ifdef ETH_10M
 enet_10m_speed();
#elif ETH_100M
 enet_100m_speed();
#endif

 // Interface selection via preprocessor conditionals
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

 // MAC_CONFIGURATION - speed and duplex mode selection
#ifdef ETH_10M
 trns_count = 6;
#ifdef HALF_DUPLEX
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x8003);
#else
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xA003);
#endif

#elif ETH_100M
#ifdef HALF_DUPLEX
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xC003);
#else
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xE003);
#endif

#else
#ifdef HALF_DUPLEX
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x0003);
#else
 write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x2003);
#endif
#endif

 LOGT("Ethernet0 RX basic test init complete");

 return 0;
}


/
 * Function: ethernet0_rx_basic_test_run
 * Description: Programs MAC, MTL, DMA registers, preloads RX descriptors, starts RX,
 * waits for transfer complete interrupts
 * Parameters:
 * cfg - pointer to test configuration
 * out - pointer to test output structure
 * Returns:
 * 0 on success (PASS), -1 on failure (FAIL)
 */
int ethernet0_rx_basic_test_run(const TestsItem *cfg, TestOutput out)
{
 (void)cfg;
 int wdata, rdata;
 int i = 0;

 if (out == 0)
 {
 LOGE("Output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("Starting Ethernet0 RX basic test run");

 // ========== MAC Register Programming (Excerpt 4) ==========

 // RX Queue Control registers
 write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL0, 0x00aa);
 write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL1, 0x000);
 write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL2, 0x08040201);
 write_enet_reg(mizar_ETHERNET0_MAC_RXQ_CTRL4, 0x0);

 // VLAN Tag Control
 write_enet_reg(mizar_ETHERNET0_MAC_VLAN_TAG_CTRL, 0x01600000);

 // MAC Packet Filter
 write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER, 0x80000400);

 // MAC Address programming - upper 16 bits [47:32]
 write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_HIGH, 0x80033607);
 write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_HIGH, 0x80022607);
 write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_HIGH, 0x80011607);
 write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH, 0x80000607);

 // MAC Address programming - lower 32 bits
 write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_LOW, 0x08090a00);
 write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_LOW, 0x08090a00);
 write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_LOW, 0x08090a00);
 write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW, 0x08090a00);

 // MAC Extended Configuration
 write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x0);

 // ========== MTL Register Programming (Excerpt 5) ==========

 // MTL TX Queue Operation Modes
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_OPERATION_MODE, 0x000f0002);
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE, 0x000f0002);
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE, 0x000f0002);
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE, 0x000f000a);

 // MTL TX Queue Quantum/Weight
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT, 0x00000014);
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT, 0x00000014);
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT, 0x00000005);

 // MTL Operation Mode
 write_enet_reg(mizar_ETHERNET0_MTL_OPERATION_MODE, 0x00000000);

 // MTL RX Queue Operation Modes - first set
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ3_OPERATION_MODE, 0x00000033);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ2_OPERATION_MODE, 0x00000033);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ1_OPERATION_MODE, 0x00000033);

 // TX Queue 0 and RX Queue 0
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_QUANTUM_WEIGHT, 0x00000005);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ0_OPERATION_MODE, 0x00000033);

 // Programming RX Queue Size to 4096 Bytes
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ3_OPERATION_MODE, 0x0000033);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ2_OPERATION_MODE, 0x0000033);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ1_OPERATION_MODE, 0x0000033);

 // Mapping of Queues to DMA Channel
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x03020100);

 // RX Queue Overflow and TX Queue Underflow Interrupt Enable
 write_enet_reg(mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS, 0x01000100);
 write_enet_reg(mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS, 0x01000100);
 write_enet_reg(mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS, 0x01000100);

 // Programming Queue Weights and Enabling Receive Queue Arbitration
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ3_CONTROL, 0x2);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ2_CONTROL, 0x4);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ1_CONTROL, 0x8);

 // ========== DMA Register Programming (Excerpt 6) ==========

 // DMA CH3/2/1 TX Control - Burst Length 16 Beats, TX Channel weight 6
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100006);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100006);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100006);

 // Programming RX Queue 0 Size to 4096 Bytes
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ0_OPERATION_MODE, 0x00f00033);

 // DMA CH3/2/1 TX Descriptor List Address
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0x00300000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0x00200000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0x00100000);

 // DMA CH3/2/1 TX Descriptor Ring Length - 32
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_RING_LENGTH, 0x0000001f);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_RING_LENGTH, 0x0000001f);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_RING_LENGTH, 0x0000001f);

 // MTL Q0 Interrupt Control Status
 write_enet_reg(mizar_ETHERNET0_MTL_Q0_INTERRUPT_CONTROL_STATUS, 0x01000100);

 // DMA CH3/2/1 Control - first set
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_CONTROL, 0x0);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_CONTROL, 0x0);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_CONTROL, 0x0);

 // DMA CH3/2/1 Control - second set (preserved as per DV source)
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_CONTROL, 0x0);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_CONTROL, 0x0);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_CONTROL, 0x0);

 // MTL RXQ0 Control
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ0_CONTROL, 0x5);

 // DMA CH3/2/1 RX Control - 16 Beats and RX Buffer size
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101000);

 // DMA System Bus Mode
 write_enet_reg(mizar_ETHERNET0_DMA_SYSBUS_MODE, 0x0103000e);

 // DMA CH3/2/1 RX Descriptor List Address
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_LIST_ADDRESS, 0x00304000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_LIST_ADDRESS, 0x00204000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_LIST_ADDRESS, 0x00104000);

 // ========== DMA CH0 TXDESC Ring Length (Excerpt 7) ==========
 // MANUAL_REVIEW: Exact placement of this excerpt relative to Excerpts 6 and 8 could not be
 // confirmed from available retrieval data. Preserving in reconstructed order.
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH, 0x0000001f);

 // DMA CH3/2/1 RX Control - 16 Beats and RX Buffer size (from Excerpt 7)
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101000);

 // ========== DMA CH1-3 RX_CONTROL2, Descriptor Preload, CH0 Setup (Excerpt 8) ==========

 // DMA CH3/2/1 RX Control2 - Receiver Descriptor Ring Length 32
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL2, 0x0000001f);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL2, 0x0000001f);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL2, 0x0000001f);

 // Preload RX descriptors
 LOGT("Preloading RX descriptors");
 preload_descriptor_rx(0xE6040000, 0xE6044000, 0x10, 0xB);

 // DMA CH0 TX Control - Burst Length 16 Beats, TX Channel weight 6
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100006);

 // DMA CH0 TX Descriptor List Address
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0xE6000000);

#ifdef DEBUG_DISPLAY
 LOGT("Power of enet_sel : %d", power);
#endif

 // Write power register
 write_reg(0XE68C2058, power);

 // DMA CH1/2/3 Interrupt Enable
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE, 0x000F9C0);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE, 0x000F9C0);
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE, 0x000F9C0);

 // ========== DMA CH0 Final Setup (Excerpt 9) ==========

 // DMA CH0 Control
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000);

 // DMA CH0 RX Control
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101000);

 // DMA CH0 RX Descriptor List Address
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS, 0xE6040000);

 // DMA CH0 RX Control2 - Receiver Descriptor Ring Length
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL2, 0x00000009);

 // DMA CH0 Interrupt Enable
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE, 0x000F9C0);

 // MAC Interrupt Enable
 write_enet_reg(mizar_ETHERNET0_MAC_INTERRUPT_ENABLE, 0x0004000);

 // DMA Mode
 write_enet_reg(mizar_ETHERNET0_DMA_MODE, 0x0000000);

 // DMA CH0 RX Control (pre-start)
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101000);

 // DMA CH0 RX Descriptor Tail Pointer
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER, 0xE6044800);

 // Start RX
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101001);

 LOGT("RX started, triggering packet stimulus");

 // DV VIP sequencer trigger - replaced with PSV logging
 // Original: write_reg(0xA0243ffc, enet_sel); // For ETH VIP sequencer start
 LOGD("DV VIP sequencer trigger: write_reg(0xA0243ffc, enet_sel)");
 // MANUAL_REVIEW: PSV must provide equivalent external packet stimulus here.
 // In DV, write_reg(0xA0243ffc, enet_sel) triggers the VIP to start sending packets.
 // In PSV, external traffic generator or loopback must be configured to send packets.

 // Original: write_reg(0xA0243ff8, 0xdeadbeef); // For ETH VIP sequencer start
 LOGD("DV VIP sequencer trigger: write_reg(0xA0243ff8, 0xdeadbeef)");
 // MANUAL_REVIEW: PSV must provide equivalent external packet stimulus here.

 // ========== Interrupt Wait Loop (Excerpt 10) ==========

 LOGT("Waiting for %u transfer complete interrupts", trns_count);

 for (i = 0; i < (int)trns_count; i++)
 {
 while (int_pend)
 {
 LOGT("--> %d Waiting for transfer complete interrupt", i);
 // PSV adaptation: wait_on(10) converted to deterministic busy-wait
 for (volatile int d = 0; d < 1000; d++);
 }
 int_pend = 1;
 }

 // PSV adaptation: wait_on(2000) converted to deterministic busy-wait
 for (volatile int d = 0; d < 200000; d++);

 LOGT("Transfer complete: received %u interrupts out of %u expected",
 irq_received_count, trns_count);

 // ========== Validation ==========

 // Runtime condition 10: Interrupt handler preservation - validate interrupt completion
 if (irq_received_count < trns_count)
 {
 LOGE("FAIL: Expected %u interrupts, received %u", trns_count, irq_received_count);
 out->status = -1;
 }

 // MANUAL_REVIEW: Runtime condition 2 (End-to-End Ethernet Validation) - Add packet content
 // verification here. Validate RX frame reception, payload integrity, source/destination MAC
 // addresses, frame length, and packet content. DV source does not contain explicit packet
 // validation - it unconditionally calls finish(0). PSV must not PASS without packet verification.

 // MANUAL_REVIEW: Runtime condition 9 (Hardware Counter Verification) - Add TX/RX packet counter
 // reads and MAC/DMA statistics validation here to supplement packet verification.

 // MANUAL_REVIEW: Runtime condition 3 (Duplex Validation) - Add MAC_CONFIGURATION readback here
 // to verify programmed duplex mode matches expected configuration.

 if (out->status == 0)
 {
 LOGT("Ethernet0 RX basic test PASSED");
 }
 else
 {
 LOGE("Ethernet0 RX basic test FAILED");
 }

 LOGT("Run complete: %s irq_received=%u irq_errors=%u",
 (out->status == 0) ? "PASS" : "FAIL",
 irq_received_count,
 irq_errors);

 return out->status;
}


/
 * Function: ethernet0_rx_basic_test_teardown
 * Description: Teardown for the Ethernet0 RX basic test
 * Parameters:
 * cfg - pointer to test configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int ethernet0_rx_basic_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("Ethernet0 RX basic test teardown: irq_received=%u irq_errors=%u",
 irq_received_count, irq_errors);

 return (irq_received_count >= trns_count && irq_errors == 0) ? 0 : -1;
}
