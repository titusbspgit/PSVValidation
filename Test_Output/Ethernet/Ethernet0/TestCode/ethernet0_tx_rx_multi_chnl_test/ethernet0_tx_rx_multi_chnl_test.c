// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "ethernet0_tx_rx_multi_chnl_test.h"
#include "test_define.inc"

// Polling loop iteration limit (from DV: for(i=0;i<20;i++))
#define POLL_LOOP_MAX 20

// Busy-wait loop bound to replace wait_on(10)
#define WAIT_DELAY_LOOPS 10000

// Packet count thresholds per progressive channel enablement
#define PKT_THRESHOLD_PHASE1 10
#define PKT_THRESHOLD_PHASE2 20
#define PKT_THRESHOLD_PHASE3 30
#define PKT_THRESHOLD_PHASE4 40

// File-scope variables preserved from DV Meta
int data_rd, data_wr;
int def_fail_cnt = 0, wr_fail_cnt = 0;
volatile int int_pend = 0;

// IRQ handler local variables
static int rdata_desc;
static int rdata_tailpointer;

// Testcase context for tracking errors
static unsigned int g_errors = 0;

// enet_sel: In DV this was derived from rand(). For PSV, use fixed selection 0.
// MANUAL_REVIEW: DV used rand() to select enet interface. PSV uses fixed enet_sel=0.
static int enet_sel = 0;

/*
 * Function: psv_busy_wait
 * Description: Deterministic busy-wait loop replacing DV wait_on(10)
 * Parameters:
 * none
 * Returns:
 * void
 /
static void psv_busy_wait(void)
{
 for (volatile int d = 0; d < WAIT_DELAY_LOOPS; d++);
}

/
 * Function: Default_IRQHandler
 * Description: Interrupt handler preserving DV RX descriptor ring maintenance,
 * DMA status clearing, and GIC IRQ clearing for CH0-CH3.
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
 LOGT("mizar_ETHERNET0_DMA_INTERRUPT_STATUS = %x", rdata);

 // GIC IRQ clearing
 GIC_ClearIRQ(42 + enet_sel);
 LOGT("Clearing Interrupt");
}

/
 * Function: ethernet0_tx_rx_multi_chnl_test_init
 * Description: Initialize GIC, select Ethernet interface, preload TX/RX
 * descriptors for all four DMA channels, configure DMA registers.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int ethernet0_tx_rx_multi_chnl_test_init(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("ethernet0_tx_rx_multi_chnl_test_init: start");

 g_errors = 0;
 int_pend = 1;

 // MANUAL_REVIEW: DV used rand() for enet_sel. PSV uses fixed enet_sel=0.
 // DV: enetsel_randvar = rand(); sel_enet = (enetsel_randvar%4)0x100000;
 enet_sel = 0;

 // GIC setup
 GIC_Set();
 GIC_EnableAllIRQ();
 LOGT("GIC initialized and all IRQs enabled");

 // Ethernet interface selection
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

 // DV debug marker: write_reg(0xA0243ffc, enet_sel)
 LOGD("DV debug marker: enet_sel = %x", enet_sel);

 // TX descriptor preloading for CH0-CH3
 LOGT("Preloading TX descriptors for CH0-CH3");
 preload_descriptor(0xE6000000, 0xE6001000, 0x3c, 0x5);
 preload_descriptor(0xE6000050, 0xE600112c, 0x5E8, 0x5);
 preload_descriptor(0xE6010000, 0xE6011000, 0x3c, 0x5);
 preload_descriptor(0xE6010050, 0xE601112c, 0x5E8, 0x5);
 preload_descriptor(0xE6020000, 0xE6021000, 0x3c, 0x5);
 preload_descriptor(0xE6020050, 0xE602112c, 0x5E8, 0x5);
 preload_descriptor(0xE6030000, 0xE6031000, 0x3c, 0x5);
 preload_descriptor(0xE6030050, 0xE603112c, 0x5E8, 0x5);

 // RX descriptor preloading for CH0-CH3
 LOGT("Preloading RX descriptors for CH0-CH3");
 preload_descriptor_rx(0xE6040000, 0xE6044000, 0x10, 0xA);
 preload_descriptor_rx(0xE6050000, 0xE6054000, 0x10, 0xA);
 preload_descriptor_rx(0xE6060000, 0xE6064000, 0x10, 0xA);
 preload_descriptor_rx(0xE6070000, 0xE6074000, 0x10, 0xA);

 // Configure TX control registers for CH0-CH3
 LOGT("Configuring TX control registers");
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100006);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100006);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100006);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100006);

 // Configure DMA CH0 control
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000);

 // Configure RX control registers for CH0-CH3
 LOGT("Configuring RX control registers");
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101000);

 // Configure TX descriptor list addresses
 LOGT("Configuring TX descriptor list addresses");
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0xE6030000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0xE6020000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0xE6010000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0xE6000000);

 // Configure RX descriptor list addresses
 LOGT("Configuring RX descriptor list addresses");
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_LIST_ADDRESS, 0xE6070000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_LIST_ADDRESS, 0xE6060000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_LIST_ADDRESS, 0xE6050000);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_LIST_ADDRESS, 0xE6040000);

 // Configure TX descriptor ring length for CH0
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH, 0x9);

 // Configure RX control2 (ring length) for CH1-CH3
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL2, 0x9);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL2, 0x9);
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL2, 0x9);

 // Configure interrupt enables for CH1-CH3
 LOGT("Configuring DMA interrupt enables");
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE, 0xf0df);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE, 0xf0df);
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE, 0xf0df);

 // Configure MTL RXQ DMA map
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000083);

 // Configure RX descriptor tail pointers
 LOGT("Configuring RX descriptor tail pointers");
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RXDESC_TAIL_POINTER, 0xE6070010);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0xE6060010);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6050010);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER, 0xE6040010);

 // Enable RX on CH3
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_RX_CONTROL, 0x00101001);

 // DV VIP sequencer start trigger
 // MANUAL_REVIEW: DV used write_reg(0xA0243ff8, 0xdeadbeef) to trigger VIP sequencer.
 // In PSV, external packet stimulus must be provided by the test environment.
 write_reg(0xA0243ff8, 0xdeadbeef);

 LOGT("ethernet0_tx_rx_multi_chnl_test_init: complete");
 return 0;
}

/
 * Function: ethernet0_tx_rx_multi_chnl_test_run
 * Description: Execute multi-channel TX/RX test with progressive channel
 * enablement and packet count validation across four phases.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * out - pointer to TestOutput for result reporting
 * Returns:
 * 0 on success, -1 on failure
 */
int ethernet0_tx_rx_multi_chnl_test_run(const TestsItem *cfg, TestOutput out)
{
 int rdata;
 int i;
 int txpkt, rxpkt;

 (void)cfg;

 if (out == 0) {
 LOGE("Output pointer is NULL");
 return -1;
 }

 out->status = 0;

 LOGT("ethernet0_tx_rx_multi_chnl_test_run: start");

 // =========================================================
 // Phase 1: Enable CH3 TX transfer, poll for 10 packets
 // =========================================================
 LOGT("Phase 1: Enable CH3 TX, threshold=%d", PKT_THRESHOLD_PHASE1);
 write_enet_reg(mizar_ETHERNET0_DMA_CH3_TX_CONTROL, 0x00100007);

 for (i = 0; i < POLL_LOOP_MAX; i++) {
 while (int_pend) {
 psv_busy_wait();
 }

 // Read current app TX descriptor and TX buffer for CH0-CH3
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_TXDESC);
 LOGT("CH0 CURRENT_APP_TXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_TXBUFFER);
 LOGT("CH0 CURRENT_APP_TXBUFFER = %x", rdata);

 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXDESC);
 LOGT("CH1 CURRENT_APP_TXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_TXBUFFER);
 LOGT("CH1 CURRENT_APP_TXBUFFER = %x", rdata);

 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_TXDESC);
 LOGT("CH2 CURRENT_APP_TXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_TXBUFFER);
 LOGT("CH2 CURRENT_APP_TXBUFFER = %x", rdata);

 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXDESC);
 LOGT("CH3 CURRENT_APP_TXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_TXBUFFER);
 LOGT("CH3 CURRENT_APP_TXBUFFER = %x", rdata);

 // Read current app RX descriptor for CH0-CH3
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH0_CURRENT_APP_RXDESC);
 LOGT("CH0 CURRENT_APP_RXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH1_CURRENT_APP_RXDESC);
 LOGT("CH1 CURRENT_APP_RXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH2_CURRENT_APP_RXDESC);
 LOGT("CH2 CURRENT_APP_RXDESC = %x", rdata);
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_CH3_CURRENT_APP_RXDESC);
 LOGT("CH3 CURRENT_APP_RXDESC = %x", rdata);

 rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
 txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
 LOGT("Phase1 poll[%d]: rxpkt=%d txpkt=%d", i, rxpkt, txpkt);

 if (rxpkt == PKT_THRESHOLD_PHASE1 && txpkt == PKT_THRESHOLD_PHASE1) break;
 int_pend = 1;
 }

 if (rxpkt != PKT_THRESHOLD_PHASE1 || txpkt != PKT_THRESHOLD_PHASE1) {
 LOGE("Phase 1 FAILED: rxpkt=%d txpkt=%d expected=%d", rxpkt, txpkt, PKT_THRESHOLD_PHASE1);
 g_errors++;
 } else {
 LOGT("Phase 1 PASSED: rxpkt=%d txpkt=%d", rxpkt, txpkt);
 }

 // Read DMA interrupt status
 rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS);
 LOGT("DMA_INTERRUPT_STATUS = %x", rdata);

 // =========================================================
 // Phase 2: Enable CH2 TX transfer, poll for 20 packets
 // =========================================================
 LOGT("Phase 2: Enable CH2 TX, threshold=%d", PKT_THRESHOLD_PHASE2);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_TX_CONTROL, 0x00100007);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000082);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RXDESC_TAIL_POINTER, 0xE6060010);
 write_enet_reg(mizar_ETHERNET0_DMA_CH2_RX_CONTROL, 0x00101001);

 // DV VIP sequencer trigger for CH2 phase
 // MANUAL_REVIEW: DV used write_reg(0xA0243ff8, 0xdeadbeee) to trigger VIP sequencer.
 write_reg(0xA0243ff8, 0xdeadbeee);

 int_pend = 1;
 for (i = 0; i < POLL_LOOP_MAX; i++) {
 while (int_pend) {
 psv_busy_wait();
 }

 rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
 txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
 LOGT("Phase2 poll[%d]: rxpkt=%d txpkt=%d", i, rxpkt, txpkt);

 if (rxpkt == PKT_THRESHOLD_PHASE2 && txpkt == PKT_THRESHOLD_PHASE2) break;
 int_pend = 1;
 }

 if (rxpkt != PKT_THRESHOLD_PHASE2 || txpkt != PKT_THRESHOLD_PHASE2) {
 LOGE("Phase 2 FAILED: rxpkt=%d txpkt=%d expected=%d", rxpkt, txpkt, PKT_THRESHOLD_PHASE2);
 g_errors++;
 } else {
 LOGT("Phase 2 PASSED: rxpkt=%d txpkt=%d", rxpkt, txpkt);
 }

 // =========================================================
 // Phase 3: Enable CH1 TX transfer, poll for 30 packets
 // =========================================================
 LOGT("Phase 3: Enable CH1 TX, threshold=%d", PKT_THRESHOLD_PHASE3);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_TX_CONTROL, 0x00100007);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000081);
 // MANUAL_REVIEW: DV RAG summary indicated CH1 tail pointer and RX control updates here.
 // Preserving pattern consistent with CH2 phase.
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RXDESC_TAIL_POINTER, 0xE6050010);
 write_enet_reg(mizar_ETHERNET0_DMA_CH1_RX_CONTROL, 0x00101001);

 // DV VIP sequencer trigger for CH1 phase
 // MANUAL_REVIEW: DV used write_reg(0xA0243ff8, 0xdeadbeed) to trigger VIP sequencer.
 write_reg(0xA0243ff8, 0xdeadbeed);

 int_pend = 1;
 for (i = 0; i < POLL_LOOP_MAX; i++) {
 while (int_pend) {
 psv_busy_wait();
 }

 rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
 txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
 LOGT("Phase3 poll[%d]: rxpkt=%d txpkt=%d", i, rxpkt, txpkt);

 if (rxpkt == PKT_THRESHOLD_PHASE3 && txpkt == PKT_THRESHOLD_PHASE3) break;
 int_pend = 1;
 }

 if (rxpkt != PKT_THRESHOLD_PHASE3 || txpkt != PKT_THRESHOLD_PHASE3) {
 LOGE("Phase 3 FAILED: rxpkt=%d txpkt=%d expected=%d", rxpkt, txpkt, PKT_THRESHOLD_PHASE3);
 g_errors++;
 } else {
 LOGT("Phase 3 PASSED: rxpkt=%d txpkt=%d", rxpkt, txpkt);
 }

 // =========================================================
 // Phase 4: Enable CH0 TX transfer, poll for 40 packets
 // =========================================================
 LOGT("Phase 4: Enable CH0 TX, threshold=%d", PKT_THRESHOLD_PHASE4);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100007);
 write_enet_reg(mizar_ETHERNET0_MTL_RXQ_DMA_MAP0, 0x00000080);
 // MANUAL_REVIEW: DV RAG summary indicated CH0 tail pointer and RX control updates here.
 // Preserving pattern consistent with CH2/CH1 phases.
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RXDESC_TAIL_POINTER, 0xE6040010);
 write_enet_reg(mizar_ETHERNET0_DMA_CH0_RX_CONTROL, 0x00101001);

 // DV VIP sequencer trigger for CH0 phase
 // MANUAL_REVIEW: DV used write_reg(0xA0243ff8, 0xdeadbeec) to trigger VIP sequencer.
 write_reg(0xA0243ff8, 0xdeadbeec);

 int_pend = 1;
 for (i = 0; i < POLL_LOOP_MAX; i++) {
 while (int_pend) {
 psv_busy_wait();
 }

 rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
 txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
 LOGT("Phase4 poll[%d]: rxpkt=%d txpkt=%d", i, rxpkt, txpkt);

 if (rxpkt == PKT_THRESHOLD_PHASE4 && txpkt == PKT_THRESHOLD_PHASE4) break;
 int_pend = 1;
 }

 if (rxpkt != PKT_THRESHOLD_PHASE4 || txpkt != PKT_THRESHOLD_PHASE4) {
 LOGE("Phase 4 FAILED: rxpkt=%d txpkt=%d expected=%d", rxpkt, txpkt, PKT_THRESHOLD_PHASE4);
 g_errors++;
 } else {
 LOGT("Phase 4 PASSED: rxpkt=%d txpkt=%d", rxpkt, txpkt);
 }

 // =========================================================
 // Final validation: end-to-end packet count verification
 // =========================================================
 rxpkt = read_enet_reg(mizar_ETHERNET0_RX_PACKETS_COUNT_GOOD_BAD);
 txpkt = read_enet_reg(mizar_ETHERNET0_TX_PACKET_COUNT_GOOD_BAD);
 LOGT("Final counters: rxpkt=%d txpkt=%d", rxpkt, txpkt);

 if (rxpkt != PKT_THRESHOLD_PHASE4) {
 LOGE("Final RX packet count mismatch: actual=%d expected=%d", rxpkt, PKT_THRESHOLD_PHASE4);
 g_errors++;
 }

 if (txpkt != PKT_THRESHOLD_PHASE4) {
 LOGE("Final TX packet count mismatch: actual=%d expected=%d", txpkt, PKT_THRESHOLD_PHASE4);
 g_errors++;
 }

 // MANUAL_REVIEW: Runtime condition requires end-to-end payload integrity,
 // MAC address, frame length, and packet content validation.
 // DV source does not provide explicit payload/MAC/frame validation logic.
 // External test environment (Wireshark/traffic analyzer) should verify these.
 // PSV PASS requires successful end-to-end packet verification beyond counters.

 // MANUAL_REVIEW: Runtime condition requires duplex mode configuration and validation.
 // DV source does not contain explicit duplex mode programming or readback.
 // ATP requires half-duplex and full-duplex validation for Ethernet.

 out->status = (g_errors == 0U) ? 0 : -1;

 LOGT("ethernet0_tx_rx_multi_chnl_test_run: %s errors=%u",
 (out->status == 0) ? "PASS" : "FAIL", g_errors);

 return out->status;
}

/
 * Function: ethernet0_tx_rx_multi_chnl_test_teardown
 * Description: Final cleanup and status reporting.
 * Parameters:
 * cfg - pointer to TestsItem configuration
 * Returns:
 * 0 on success, -1 on failure
 */
int ethernet0_tx_rx_multi_chnl_test_teardown(const TestsItem *cfg)
{
 (void)cfg;

 LOGT("ethernet0_tx_rx_multi_chnl_test_teardown: errors=%u", g_errors);
 return (g_errors == 0U) ? 0 : -1;
}
