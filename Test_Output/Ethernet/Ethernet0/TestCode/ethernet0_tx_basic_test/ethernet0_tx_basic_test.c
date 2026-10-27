// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_tx_basic_test.h"
#include "test_define.inc"

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
// ethernet0_programming_sequence.h. If not provided by those headers, uncomment definitions below.
// extern int enet_select;
// extern int enet_sel;
// extern int power;

// Tracking variable for interrupt-based validation
static unsigned int irq_received_count = 0;


/*
 * Function: Default_IRQHandler
 * Description: Ethernet interrupt handler - reads DMA interrupt status, clears DMA CH0 status,
 * re-reads DMA interrupt status, writes power register, clears GIC IRQ
 * Parameters:
 * none
 * Returns:
 * void
 */
void Default_IRQHandler(void)
{
 int rdata, wdata;
 unsigned int irq_no;
 int_pend = 0;
 irq_no = 42 + enet_select;
 enet_sel = enet_select;

#ifdef DEBUG_DISPLAY
 LOGT("ENET IRQ NO = %x", irq_no);
 LOGT("printing pointers :");
#endif

 // Read DMA interrupt status (first read)
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);

#ifdef DEBUG_DISPLAY
 LOGT("mizar_ETHERNET0_DMA_INTERRUPT_STATUS = %x", rdata);
 LOGT("clearing individual dma int status :");
#endif

 // Clear DMA CH0 status by writing 0xffffffff
 wdata = 0xffffffff;
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_STATUS, wdata);

#ifdef DEBUG_DISPLAY
 LOGT("printing dma int status :");
#endif

 // Re-read DMA interrupt status after clearing (second read)
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);

#ifdef DEBUG_DISPLAY
 LOGT("mizar_ETHERNET0_DMA_INTERRUPT_STATUS = %x", rdata);
#endif

 // Commented-out DMA channel status reads preserved from DV source
 // rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_STATUS);
 // LOGT("mizar_ETHERNET0_DMA_CH0_STATUS = %x", rdata);
 // rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_STATUS);
 // LOGT("mizar_ETHERNET0_DMA_CH1_STATUS = %x", rdata);
 // rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_STATUS);
 // LOGT("mizar_ETHERNET0_DMA_CH2_STATUS = %x", rdata);
 // rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_STATUS);
 // LOGT("mizar_ETHERNET0_DMA_CH3_STATUS = %x", rdata);

 // Write power register
 write_reg(0XE68C2050, power);

 // Clear GIC IRQ
#ifdef CLRIRQ_42
 GIC_ClearIRQ(42);
#else
 GIC_ClearIRQ(irq_no);
#endif

 irq_received_count++;
}


/*
 * Function: ethernet0_tx_basic_test_init
 * Description: Initializes GIC, selects Ethernet interface and speed/duplex mode,
 * calls non_secure_prot_nic, programs MAC address registers
 * Parameters:
 * cfg - pointer to test configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int ethernet0_tx_basic_test_init(const TestsItem *cfg)
{
 (void)cfg;

 int_pend = 1;
 def_fail_cnt = 0;
 wr_fail_cnt = 0;
 irq_received_count = 0;

 int wdata, rdata, i;
 int address, read;

 LOGT("Ethernet0 TX basic test init - GIC and interface setup");

 // GIC initialization
 GIC_Set();
 GIC_EnableAllIRQ();

 // Speed selection via preprocessor conditionals
#ifdef ETH_10M
 enet_10m_speed();
#elif ETH_100M
 enet_100m_speed();
#endif

 // trns_count adjustment for 10M speed
#ifdef ETH_10M
 trns_count = 6;
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

 // non_secure_prot_nic call
 non_secure_prot_nic();

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

 LOGT("Ethernet0 TX basic test init complete");

 return 0;
}


/* Function: ethernet0_tx_basic_test_run
 * Description: Programs MAC, MTL, DMA registers, preloads TX descriptors, starts TX,
 * waits for transfer complete interrupts
 * Parameters:
 * cfg - pointer to test configuration
 * out - pointer to test output structure
 * Returns:
 * 0 on success (PASS), -1 on failure (FAIL)
 */
int ethernet0_tx_basic_test_run(const TestsItem *cfg, TestOutput *out)
{
 (void)cfg;
 int wdata, rdata, i;
 int address, read;

 if (out == 0)
 {
 LOGE("Output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("Starting Ethernet0 TX basic test run");

 // ========== MAC Packet Filter ==========
 write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER, 0x80000400);

 // ========== MAC_CONFIGURATION - speed and duplex mode selection ==========
#ifdef ETH_10M
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

 // ========== VIP Sequencer Start - DV-only triggers ==========
 // Original: write_reg(0xA0243ffc, enet_sel); // For ETH VIP sequencer start
 LOGD("DV VIP sequencer trigger: write_reg(0xA0243ffc, enet_sel)");
 // MANUAL_REVIEW: PSV must provide equivalent external packet stimulus here.
 // In DV, write_reg(0xA0243ffc, enet_sel) triggers the VIP to start sending/receiving packets.
 // In PSV, external traffic generator or loopback must be configured.

 // Original: write_reg(0xA0243ff8, 0xdeadbeef); // For ETH VIP sequencer start
 LOGD("DV VIP sequencer trigger: write_reg(0xA0243ff8, 0xdeadbeef)");
 // MANUAL_REVIEW: PSV must provide equivalent external packet stimulus here.

 // ========== MAC Extended Configuration ==========
 write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x0);

 // ========== MTL Register Programming ==========

 // MTL TX Queue Operation Modes
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_OPERATION_MODE, 0x000f000a);
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE, 0x000f000a);
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE, 0x000f000a);
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE, 0x000f000a);

 // MTL TX Queue Quantum/Weight
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT, 0x00000005);
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT, 0x00000005);
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT, 0x00000005);
 write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_QUANTUM_WEIGHT, 0x00000005);

 // RX Queue Overflow and TX Queue Underflow Interrupt Enable
 write_enet_reg(mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS, 0x01000100);
 write_enet_reg(mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS, 0x01000100);
 write_enet_reg(mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS, 0x01000100);
 write_enet_reg(mizar_ETHERNET0_MTL_Q0_INTERRUPT_CONTROL_STATUS, 0x01000100);

 // ========== DMA Register Programming ==========

 // DMA System Bus Mode
 write_enet_reg(mizar_ETHERNET0_DMA_SYSBUS_MODE, 0x0103000e);

 // Preload TX descriptors
 LOGT("Preloading TX descriptors");
 preload_descriptor(0xE6000000, 0xE6008000, 0x3c, 0x5);
 preload_descriptor(0xE6000050, 0xE600812c, 0x5E8, 0x5);
 // Commented-out preload_descriptor calls preserved from DV source
 // preload_descriptor(0xE6000000, 0xE600812C, 0x3c, 0x5);
 // preload_descriptor(0x00200000, 0x00208000, 0xe, 0x60);
 // preload_descriptor(0x00100000, 0x00108000, 0xe, 0x60);
 // preload_descriptor(0x00000000, 0x00008000, 0xe, 0x60);

 // DMA TX Descriptor Ring Length
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_RING_LENGTH, 0x000000A);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_RING_LENGTH, 0x000000A);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_RING_LENGTH, 0x000000A);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH, 0x000000A);

 // DMA TX Descriptor List Address
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0x00300000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0x00200000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0x00100000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0xE6000000);

 // DMA TX Descriptor Tail Pointer
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0x00309730);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0x00209730);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER, 0x00109730);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER, 0xE6009EB4);

 // DMA Channel Control
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_CONTROL, 0x0);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_CONTROL, 0x0);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_CONTROL, 0x0);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000);

 // DMA Interrupt Enable
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE, 0x000F0C7);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE, 0x000F0C7);
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE, 0x000F0C7);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE, 0x000F0C7);

 // Enabling HSS Autoreg Ethernet Interrupts
#ifdef DEBUG_DISPLAY
 LOGT("Power of enet_sel : %d", power);
#endif
 write_reg(0XE68C2058, power);

 // START TX - only CH0 enabled (CH1-3 commented out in DV source)
 // write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100007);
 // write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100007);
 // write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100007);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100007);

 LOGT("TX started on DMA CH0, waiting for %u transfer complete interrupts", trns_count);

 // ========== Interrupt Wait Loop ==========
 for (i = 0; i < (int)trns_count; i++)
 {
 while (int_pend)
 {
#ifdef DEBUG_DISPLAY
 LOGT("---->%d trnsf intrpt", i);
#endif
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

 // MANUAL_REVIEW: Runtime condition 2 (End-to-End Ethernet Validation) - Add TX frame
 // verification here. Validate TX frame transmission, payload integrity, source/destination MAC
 // addresses, frame length, and packet content. DV source does not contain explicit packet
 // validation - it unconditionally calls finish(0). PSV must not PASS without packet verification.

 // MANUAL_REVIEW: Runtime condition 9 (Hardware Counter Verification) - Add TX packet counter
 // reads and MAC/DMA statistics validation here to supplement packet verification.

 // MANUAL_REVIEW: Runtime condition 3 (Duplex Validation) - Add MAC_CONFIGURATION readback here
 // to verify programmed duplex mode matches expected configuration.

 if (out->status == 0)
 {
 LOGT("Ethernet0 TX basic test PASSED");
 }
 else
 {
 LOGE("Ethernet0 TX basic test FAILED");
 }

 LOGT("Run complete: %s irq_received=%u",
 (out->status == 0) ? "PASS" : "FAIL",
 irq_received_count);

 return out->status;
}


/* Function: ethernet0_tx_basic_test_teardown
 * Description: Teardown for the Ethernet0 TX basic test
 * Parameters:
 * cfg - pointer to test configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int ethernet0_tx_basic_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("Ethernet0 TX basic test teardown: irq_received=%u",
 irq_received_count);

 return (irq_received_count >= trns_count) ? 0 : -1;
}
