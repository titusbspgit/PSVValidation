// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_tx_rx_multi_chnl_test.h"
#include "test_define.inc"

// Testcase context structure
typedef struct {
 unsigned int errors;
 unsigned int tx_validated;
 unsigned int rx_validated;
 unsigned int pkt_integrity_pass;
 unsigned int irq_serviced_count;
} enet0_multi_chnl_ctx_t;

static enet0_multi_chnl_ctx_t g_ctx;

// File-scope variables preserved from DV source
int data_rd, data_wr;
int def_fail_cnt = 0, wr_fail_cnt = 0;
volatile int int_pend = 1;
int enet_sel = 0;

// IRQ handler local variables at file scope for ISR access
static int rdata_desc;
static int rdata_tailpointer;

/*
 * Function: Default_IRQHandler
 * Description: Interrupt handler for Ethernet0 DMA channels.
 * Manages RX descriptor ring for CH0-CH3,
 * clears DMA channel status, reads DMA interrupt
 * status, and clears GIC IRQ.
 * Parameters:
 * none
 * Returns:
 * void
 /
void Default_IRQHandler(void)
{
 int wdata;
 int rdata;

 int_pend = 0;
 g_ctx.irq_serviced_count++;
 LOGT("Default_IRQHandler entered");

 // RX descriptor tail pointer management for CH0
 rdata_desc = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC);
 rdata_tailpointer = read_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER);
 if (rdata_tailpointer == rdata_desc) {
 if (rdata_tailpointer == (int)0xE60400f0) {
#ifdef PRELOAD_AGAIN
 preload_descriptor_rx(0xE6040000, 0xE6044000, 0x10, 0x10);
#endif
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER, 0xE6040000);
 } else {
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER, (rdata_tailpointer + 0x10));
 }
 }

 // RX descriptor tail pointer management for CH1
 rdata_desc = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_RXDESC);
 rdata_tailpointer = read_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER);
 if (rdata_tailpointer == rdata_desc) {
 if (rdata_tailpointer == (int)0xE60500f0) {
#ifdef PRELOAD_AGAIN
 preload_descriptor_rx(0xE6050000, 0xE6054000, 0x10, 0x10);
#endif
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6050000);
 } else {
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, (rdata_tailpointer + 0x10));
 }
 }

 // RX descriptor tail pointer management for CH2
 rdata_desc = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXDESC);
 LOGT("mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXDESC = %x", rdata_desc);
 rdata_tailpointer = read_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER);
 if (rdata_tailpointer == rdata_desc) {
 if (rdata_tailpointer == (int)0xE60600f0) {
#ifdef PRELOAD_AGAIN
 preload_descriptor_rx(0xE6060000, 0xE6064000, 0x10, 0x10);
#endif
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0xE6060000);
 } else {
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, (rdata_tailpointer + 0x10));
 }
 }

 // RX descriptor tail pointer management for CH3
 rdata_desc = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_RXDESC);
 rdata_tailpointer = read_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER);
 if (rdata_tailpointer == rdata_desc) {
 if (rdata_tailpointer == (int)0xE60700f0) {
#ifdef PRELOAD_AGAIN
 preload_descriptor_rx(0xE6070000, 0xE6074000, 0x10, 0x10);
#endif
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0xE6070000);
 } else {
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, (rdata_tailpointer + 0x10));
 }
 }

 // Clear DMA channel status registers
 wdata = 0xffffffff;
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_STATUS, wdata);
 wdata = 0xffffffff;
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_STATUS, wdata);
 wdata = 0xffffffff;
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_STATUS, wdata);
 wdata = 0xffffffff;
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_STATUS, wdata);

 // Read DMA interrupt status
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);
 LOGT("DMA_INTERRUPT_STATUS = %x", rdata);

 // GIC IRQ clearing
 GIC_ClearIRQ(42 + enet_sel);
 LOGT("Clearing Interrupt");
}

/
 * Function: ethernet0_tx_rx_multi_chnl_test_init
 * Description: Initializes GIC, selects Ethernet interface,
 * preloads TX and RX descriptors for all 4 DMA channels,
 * and configures DMA control registers.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int ethernet0_tx_rx_multi_chnl_test_init(const TestsItem *cfg)
{
 (void)cfg;

 g_ctx = (enet0_multi_chnl_ctx_t){0};

 LOGT("ethernet0_tx_rx_multi_chnl_test init start");

 // DV source: int_pend = 1
 int_pend = 1;

 // DV source: enetsel_randvar = rand(); sel_enet = (enetsel_randvar%4)0x100000;
 // rand() is prohibited in PSV. Using deterministic value.
 // Selecting enet_sel = 0 (ENET0) deterministically.
 enet_sel = 0;

 // GIC setup
 GIC_Set();
 GIC_EnableAllIRQ();
 LOGT("GIC initialized and all IRQs enabled");

 // Conditional interface selection
#ifdef SEL_ENET0
 enet_intf_sel(0);
#elif defined(SEL_ENET1)
 enet_intf_sel(1);
#elif defined(SEL_ENET2)
 enet_intf_sel(2);
#elif defined(SEL_ENET3)
 enet_intf_sel(3);
#else
 enet_intf_sel(4);
#endif

 // DV marker: write_reg(0xA0243ffc, enet_sel)
 // This is the DV debug marker address 0xA0243ffc - convert to PSV logging
 LOGD("DV debug marker: enet_sel = %x", enet_sel);

 // TX descriptor preloading for CH0
 LOGT("Preloading TX descriptors for CH0");
 preload_descriptor(0xE6000000, 0xE6001000, 0x3c, 0x5);
 preload_descriptor(0xE6000050, 0xE600112c, 0x5E8, 0x5);

 // TX descriptor preloading for CH1
 LOGT("Preloading TX descriptors for CH1");
 preload_descriptor(0xE6010000, 0xE6011000, 0x3c, 0x5);
 preload_descriptor(0xE6010050, 0xE601112c, 0x5E8, 0x5);

 // TX descriptor preloading for CH2
 LOGT("Preloading TX descriptors for CH2");
 preload_descriptor(0xE6020000, 0xE6021000, 0x3c, 0x5);
 preload_descriptor(0xE6020050, 0xE602112c, 0x5E8, 0x5);

 // TX descriptor preloading for CH3
 LOGT("Preloading TX descriptors for CH3");
 preload_descriptor(0xE6030000, 0xE6031000, 0x3c, 0x5);
 preload_descriptor(0xE6030050, 0xE603112c, 0x5E8, 0x5);

 // RX descriptor preloading for CH0-CH3
 LOGT("Preloading RX descriptors for CH0-CH3");
 preload_descriptor_rx(0xE6040000, 0xE6044000, 0x10, 0xA);
 preload_descriptor_rx(0xE6050000, 0xE6054000, 0x10, 0xA);
 preload_descriptor_rx(0xE6060000, 0xE6064000, 0x10, 0xA);
 preload_descriptor_rx(0xE6070000, 0xE6074000, 0x10, 0xA);

 LOGT("Descriptor preloading complete");

 // Configure TX control registers
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100006);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100006);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100006);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100006);

 // Configure DMA CH0 control
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000);

 // Configure RX control registers
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101000);

 // Configure TX descriptor list addresses
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0xE6030000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0xE6020000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0xE6010000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0xE6000000);

 // Configure RX descriptor list addresses
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_LIST_ADDRESS, 0xE6070000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_LIST_ADDRESS, 0xE6060000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_LIST_ADDRESS, 0xE6050000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS, 0xE6040000);

 // Configure TX descriptor ring length
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH, 0x9);

 // Configure RX control2 (ring lengths) for CH1-CH3
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL2, 0x9);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL2, 0x9);
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL2, 0x9);

 // Configure interrupt enables for CH1-CH3
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE, 0xf0df);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE, 0xf0df);
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE, 0xf0df);

 // Configure MTL RXQ DMA map
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000083);

 // Configure RX descriptor tail pointers
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0xE6070010);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0xE6060010);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6050010);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER, 0xE6040010);

 // Enable RX on CH3
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101001);

 // ETH VIP sequencer start trigger
 write_reg(0xA0243ff8, 0xdeadbeef);
 // MANUAL_REVIEW: write_reg(0xA0243ff8, 0xdeadbeef) is a VIP sequencer trigger.
 // In PSV, ensure external packet stimulus is provided by the test environment.

 LOGT("ethernet0_tx_rx_multi_chnl_test init complete");
 return 0;
}

/
 * Function: ethernet0_tx_rx_multi_chnl_test_run
 * Description: Executes multi-channel TX/RX test with progressive
 * channel enablement and packet count validation.
 * Validates end-to-end Ethernet traffic across CH0-CH3.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * out - pointer to TestOutput for results
 * Returns:
 * 0 on PASS, -1 on FAIL
 */
int ethernet0_tx_rx_multi_chnl_test_run(const TestsItem *cfg, TestOutput out)
{
 int wdata, rdata, i, txpkt, rxpkt;
 int address, read;

 (void)cfg;
 (void)address;
 (void)read;

 if (out == 0) {
 LOGE("Output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("ethernet0_tx_rx_multi_chnl_test run start");

 // Read CH1 TXDESC and TXBUFFER
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXDESC);
 LOGT("DMA_CH1_CURRENT_APP_TXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXBUFFER);
 LOGT("DMA_CH1_CURRENT_APP_TXBUFFER = %x", rdata);

 // Read CH3 TXDESC and TXBUFFER
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXDESC);
 LOGT("DMA_CH3_CURRENT_APP_TXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXBUFFER);
 LOGT("DMA_CH3_CURRENT_APP_TXBUFFER = %x", rdata);

 // Read DMA interrupt status
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);
 LOGT("DMA_INTERRUPT_STATUS = %x", rdata);

 // ============================================================
 // Phase 1: CH3 TX transfer start and polling loop - threshold 10
 // ============================================================
 LOGT("Phase 1: Enable CH3 TX, target 10 packets");
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100007);
 for (i = 0; i < 20; i++) {
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 }
 // Read CURRENT_APP_TXDESC for CH0-CH3
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_TXDESC);
 LOGT("CH0 TXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_TXBUFFER);
 LOGT("CH0 TXBUFFER = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXDESC);
 LOGT("CH1 TXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXBUFFER);
 LOGT("CH1 TXBUFFER = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_TXDESC);
 LOGT("CH2 TXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_TXBUFFER);
 LOGT("CH2 TXBUFFER = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXDESC);
 LOGT("CH3 TXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXBUFFER);
 LOGT("CH3 TXBUFFER = %x", rdata);
 // Read CURRENT_APP_RXDESC for CH0-CH3
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC);
 LOGT("CH0 RXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_RXDESC);
 LOGT("CH1 RXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXDESC);
 LOGT("CH2 RXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_RXDESC);
 LOGT("CH3 RXDESC = %x", rdata);
 rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
 txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
 LOGT("Phase1 poll i=%d rxpkt=%d txpkt=%d", i, rxpkt, txpkt);
 if (rxpkt == 10 && txpkt == 10) break;
 int_pend = 1;
 }
 if (rxpkt != 10 || txpkt != 10) {
 LOGE("Phase 1 FAIL: expected rxpkt=10 txpkt=10, got rxpkt=%d txpkt=%d", rxpkt, txpkt);
 g_ctx.errors++;
 } else {
 LOGT("Phase 1 PASS: rxpkt=%d txpkt=%d", rxpkt, txpkt);
 g_ctx.tx_validated++;
 g_ctx.rx_validated++;
 }

 // ============================================================
 // Phase 2: CH2 enable and polling loop - threshold 20
 // ============================================================
 LOGT("Phase 2: Enable CH2 TX, target 20 packets");
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100007);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000082);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0xE6060010);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101001);
 write_reg(0xA0243ff8, 0xdeadbeee);
 // MANUAL_REVIEW: write_reg(0xA0243ff8, 0xdeadbeee) is a VIP sequencer trigger.
 // In PSV, ensure external packet stimulus is provided by the test environment.
 int_pend = 1;
 for (i = 0; i < 20; i++) {
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 }
 rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
 txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
 LOGT("Phase2 poll i=%d rxpkt=%d txpkt=%d", i, rxpkt, txpkt);
 if (rxpkt == 20 && txpkt == 20) break;
 int_pend = 1;
 }
 if (rxpkt != 20 || txpkt != 20) {
 LOGE("Phase 2 FAIL: expected rxpkt=20 txpkt=20, got rxpkt=%d txpkt=%d", rxpkt, txpkt);
 g_ctx.errors++;
 } else {
 LOGT("Phase 2 PASS: rxpkt=%d txpkt=%d", rxpkt, txpkt);
 g_ctx.tx_validated++;
 g_ctx.rx_validated++;
 }

 // ============================================================
 // Phase 3: CH1 enable and polling loop - threshold 30
 // ============================================================
 LOGT("Phase 3: Enable CH1 TX, target 30 packets");
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100007);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000081);
 // MANUAL_REVIEW: RAG summary indicated CH1 RX tail pointer and RX control updates here.
 // Preserving pattern consistent with CH2 enable sequence.
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6050010);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101001);
 write_reg(0xA0243ff8, 0xdeadbeed);
 // MANUAL_REVIEW: write_reg(0xA0243ff8, 0xdeadbeed) is a VIP sequencer trigger.
 // In PSV, ensure external packet stimulus is provided by the test environment.
 int_pend = 1;
 for (i = 0; i < 20; i++) {
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 }
 rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
 txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
 LOGT("Phase3 poll i=%d rxpkt=%d txpkt=%d", i, rxpkt, txpkt);
 if (rxpkt == 30 && txpkt == 30) break;
 int_pend = 1;
 }
 if (rxpkt != 30 || txpkt != 30) {
 LOGE("Phase 3 FAIL: expected rxpkt=30 txpkt=30, got rxpkt=%d txpkt=%d", rxpkt, txpkt);
 g_ctx.errors++;
 } else {
 LOGT("Phase 3 PASS: rxpkt=%d txpkt=%d", rxpkt, txpkt);
 g_ctx.tx_validated++;
 g_ctx.rx_validated++;
 }

 // ============================================================
 // Phase 4: CH0 enable and polling loop - threshold 40
 // ============================================================
 LOGT("Phase 4: Enable CH0 TX, target 40 packets");
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100007);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000080);
 // MANUAL_REVIEW: RAG summary indicated CH0 RX tail pointer and RX control updates here.
 // Preserving pattern consistent with CH2 enable sequence.
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER, 0xE6040010);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101001);
 write_reg(0xA0243ff8, 0xdeadbeec);
 // MANUAL_REVIEW: write_reg(0xA0243ff8, 0xdeadbeec) is a VIP sequencer trigger.
 // In PSV, ensure external packet stimulus is provided by the test environment.
 int_pend = 1;
 for (i = 0; i < 20; i++) {
 while (int_pend) {
 for (volatile int d = 0; d < 10000; d++);
 }
 rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
 txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
 LOGT("Phase4 poll i=%d rxpkt=%d txpkt=%d", i, rxpkt, txpkt);
 if (rxpkt == 40 && txpkt == 40) break;
 int_pend = 1;
 }
 if (rxpkt != 40 || txpkt != 40) {
 LOGE("Phase 4 FAIL: expected rxpkt=40 txpkt=40, got rxpkt=%d txpkt=%d", rxpkt, txpkt);
 g_ctx.errors++;
 } else {
 LOGT("Phase 4 PASS: rxpkt=%d txpkt=%d", rxpkt, txpkt);
 g_ctx.tx_validated++;
 g_ctx.rx_validated++;
 }

 // ============================================================
 // Final validation
 // ============================================================
 // MANUAL_REVIEW: Runtime condition requires end-to-end packet verification
 // including payload integrity, MAC addresses, frame length.
 // The DV source validates via packet counters and interrupt-driven
 // descriptor management. Full payload/MAC/frame validation requires
 // additional infrastructure not present in the retrieved DV source.
 // Preserving counter-based validation from DV and flagging additional
 // requirements.

 // Validate TX counters
 txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
 LOGT("Final TX_PACKET_COUNT_GOOD_BAD = %d", txpkt);
 if (txpkt != 40) {
 LOGE("Final TX packet count mismatch: expected 40, got %d", txpkt);
 g_ctx.errors++;
 }

 // Validate RX counters
 rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
 LOGT("Final RX_PACKETS_COUNT_GOOD_BAD = %d", rxpkt);
 if (rxpkt != 40) {
 LOGE("Final RX packet count mismatch: expected 40, got %d", rxpkt);
 g_ctx.errors++;
 }

 // Validate interrupt servicing occurred
 if (g_ctx.irq_serviced_count == 0) {
 LOGE("No interrupts were serviced during test");
 g_ctx.errors++;
 } else {
 LOGT("Total interrupts serviced: %u", g_ctx.irq_serviced_count);
 }

 // Validate all 4 phases passed TX validation
 if (g_ctx.tx_validated != 4) {
 LOGE("TX validation incomplete: %u of 4 phases passed", g_ctx.tx_validated);
 g_ctx.errors++;
 }

 // Validate all 4 phases passed RX validation
 if (g_ctx.rx_validated != 4) {
 LOGE("RX validation incomplete: %u of 4 phases passed", g_ctx.rx_validated);
 g_ctx.errors++;
 }

 // MANUAL_REVIEW: End-to-end payload integrity, source/destination MAC address,
 // frame length, and packet content validation require external packet capture
 // infrastructure not present in the retrieved DV source. The DV source relies
 // on VIP-driven packet generation and counter-based validation.
 // Ensure PSV test environment provides equivalent external packet stimulus
 // and capture capability for full end-to-end validation.

 // MANUAL_REVIEW: Duplex mode configuration and validation was not present
 // in the retrieved DV source. Runtime condition requires duplex validation.
 // Add duplex configuration and readback verification as required by ATP.

 out->status = (g_ctx.errors == 0U) ? 0 : -1;

 LOGT("Run complete: %s errors=%u tx_validated=%u rx_validated=%u irq_count=%u",
 (out->status == 0) ? "PASS" : "FAIL",
 g_ctx.errors,
 g_ctx.tx_validated,
 g_ctx.rx_validated,
 g_ctx.irq_serviced_count);

 return out->status;
}

/
 * Function: ethernet0_tx_rx_multi_chnl_test_teardown
 * Description: Final cleanup and status reporting.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on PASS, -1 on FAIL
 */
int ethernet0_tx_rx_multi_chnl_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("ethernet0_tx_rx_multi_chnl_test teardown: errors=%u", g_ctx.errors);
 return g_ctx.errors == 0U ? 0 : -1;
}
