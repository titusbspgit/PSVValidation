#!/usr/bin/env python3
"""Self-contained Excel workbook generator for Ethernet TestPlan.
This script embeds all test data and generates a properly formatted XLSX workbook.
Usage: python3 create_workbook.py
"""
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
except ImportError:
    print("Installing openpyxl...")
    os.system(f"{sys.executable} -m pip install openpyxl")
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side

# ============================================================
# EMBEDDED TEST DATA - EXACT VALUES FROM JSON
# ============================================================

TEST_DATA = [
    {
        "Index": "3",
        "SS / Module": "NA",
        "Feature": "NA",
        "Test Case Name": "ethernet0_tx_basic_test",
        "Test Description": "Basic Ethernet0 transmit test that configures MAC addresses, packet filter, MAC configuration for speed and duplex mode, MTL TX queue operation modes and weights, DMA system bus mode, TX descriptors, DMA channel ring lengths, list addresses, tail pointers, channel controls, and interrupt enables. The test starts TX on DMA channel 0, waits for a configurable number of transfer-complete interrupts, and verifies successful completion. The interrupt handler clears DMA channel status and GIC IRQ on each interrupt.",
        "Speed": "ETH_10M (10M, conditional); ETH_100M (100M, conditional); default (1G, no ETH_10M/ETH_100M defined)",
        "Mode": "HALF_DUPLEX (conditional); Full Duplex (default, no HALF_DUPLEX defined)",
        "Memory Start Offset": "0xE6000000; 0xE6008000; 0xE6000050; 0xE600812c",
        "Memory End Offset": "NA",
        "Remarks": "The testcase uses conditional compilation for speed (ETH_10M, ETH_100M, default 1G), duplex mode (HALF_DUPLEX or Full Duplex), Ethernet interface selection (SEL_ENET0 through SEL_ENET3, default 4), and IRQ clearing (CLRIRQ_42). External functions (GIC_Set, GIC_EnableAllIRQ, GIC_ClearIRQ, enet_10m_speed, enet_100m_speed, enet_intf_sel, non_secure_prot_nic, write_enet_reg, read_enet_reg, write_reg, preload_descriptor, wait_on, finish) are defined in external headers and are not available for local inspection. External global variables (int_pend, enet_sel, enet_select, power) are used but defined externally. Agent 4 mappings were empty, so no canonical register names are available for the Impacted Registers field. The while(int_pend) polling loop has no explicit timeout or fail path in the source; if an interrupt does not arrive, the test will hang.",
        "Test Steps / Procedure": """1. Initialize GIC and enable all interrupts.
2. Configure Ethernet speed (10M/100M/1G) based on build configuration.
3. Select Ethernet interface instance.
4. Configure non-secure protection via NIC.
5. Program MAC address registers (high and low) for addresses 0 through 3 with address enable and DMA channel selection.
6. Configure MAC packet filter for hash filtering and receive-all mode.
7. Configure MAC for TX and RX enable with appropriate speed and duplex mode.
8. Signal ETH VIP sequencer start.
9. Clear MAC extended configuration.
10. Program MTL TX queue operation modes for queues 0-3 with 4096-byte queue size and store-and-forward.
11. Program MTL TX queue quantum/weight values for queues 0-3.
12. Enable MTL queue interrupt control status for RX overflow and TX underflow on queues 0-3.
13. Configure DMA system bus mode with outstanding request limits and burst selection.
14. Preload TX descriptors for DMA channel 0.
15. Program DMA TX descriptor ring lengths for channels 0-3.
16. Program DMA TX descriptor list base addresses for channels 0-3.
17. Program DMA TX descriptor tail pointers for channels 0-3.
18. Clear DMA channel control registers for channels 0-3.
19. Enable DMA channel interrupts for channels 0-3.
20. Enable HSS Autoreg Ethernet interrupts.
21. Start TX on DMA channel 0.
22. Wait for each transfer-complete interrupt (10 iterations default, 6 for 10M speed), polling interrupt pending flag with wait timeout.
23. Perform final wait after all transfers complete.
24. Complete test with pass status.
25. ISR: Clear interrupt pending flag, read and clear DMA interrupt status, write HSS autoreg, clear GIC IRQ.""",
        "Impacted Registers": "NA",
        "Validation / Acceptance Criteria": """1. Each transmit transfer must trigger a DMA interrupt that is successfully received and acknowledged by the interrupt handler.
2. The DMA interrupt status register must be readable and clearable after each interrupt.
3. All configured transfer iterations must complete with interrupt-driven synchronization.
4. The GIC IRQ must be properly cleared after each interrupt service.
5. The test completes successfully with a pass indication after all transfers and a final wait period.""",
        "Meta Test Description": "This testcase performs a basic Ethernet0 transmit (TX) operation. It initializes the GIC interrupt controller, configures the Ethernet MAC address registers (addresses 0-3 high and low), sets the MAC packet filter for hash filtering and receive-all, configures the MAC for TX/RX enable with speed and duplex mode selected via conditional compilation (ETH_10M/ETH_100M/default 1G, HALF_DUPLEX/Full Duplex). It signals the ETH VIP sequencer via two hardcoded write_reg calls. It programs the MTL TX queue operation modes for queues 0-3 (4096 bytes, store-and-forward), sets quantum/weight values for queues 0-3, enables MTL queue interrupt control status for overflow/underflow on queues 0-3. It configures DMA system bus mode, preloads TX descriptors via preload_descriptor() calls, programs DMA TX descriptor ring lengths for channels 0-3, sets TX descriptor list base addresses for channels 0-3, sets TX descriptor tail pointers for channels 0-3, clears DMA channel control for channels 0-3, enables DMA channel interrupts for channels 0-3, enables HSS Autoreg Ethernet interrupts, then starts TX on DMA channel 0. The test then enters a loop for trns_count iterations (10 by default, 6 for ETH_10M), waiting for each transfer interrupt via polling int_pend with wait_on(10), resetting int_pend after each interrupt. After all transfers complete, it waits wait_on(2000) and calls finish(0). The Default_IRQHandler clears int_pend, reads DMA_INTERRUPT_STATUS, writes 0xFFFFFFFF to DMA_CH0_STATUS to clear status, reads DMA_INTERRUPT_STATUS again, writes HSS autoreg, and clears the GIC IRQ.",
        "Meta Test Steps / Procedure": """1. Set global int_pend = 1.
2. Declare local variables: int wdata, rdata, i; int address, read.
3. Call GIC_Set() to initialize the GIC.
4. Call GIC_EnableAllIRQ() to enable all interrupts.
5. [Conditional ETH_10M] Call enet_10m_speed().
6. [Conditional ETH_100M] Call enet_100m_speed().
7. [Conditional ETH_10M] Set trns_count = 6.
8. [Conditional SEL_ENET0] Call enet_intf_sel(0).
9. [Conditional SEL_ENET1] Call enet_intf_sel(1).
10. [Conditional SEL_ENET2] Call enet_intf_sel(2).
11. [Conditional SEL_ENET3] Call enet_intf_sel(3).
12. [Else default] Call enet_intf_sel(4).
13. Call non_secure_prot_nic().
14. write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_HIGH, 0x80033607) \u2014 addr_en, dma_channel_sel=3, addr[47:32]=3607.
15. write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_HIGH, 0x80022607) \u2014 addr_en, dma_channel_sel=2, addr[47:32]=2607.
16. write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_HIGH, 0x80011607).
17. write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_HIGH, 0x80000607).
18. write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS3_LOW, 0x08090a00) \u2014 addr[31:0].
19. write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS2_LOW, 0x08090a00).
20. write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS1_LOW, 0x08090a00).
21. write_enet_reg(mizar_ETHERNET0_MAC_ADDRESS0_LOW, 0x08090a00).
22. write_enet_reg(mizar_ETHERNET0_MAC_PACKET_FILTER, 0x80000400) \u2014 Hash Filter Enabled, Receive All.
23. [Conditional ETH_10M && HALF_DUPLEX] write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x8003) \u2014 TX/RX enable, Half Duplex, 10M speed.
24. [Conditional ETH_10M && !HALF_DUPLEX] write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xA003) \u2014 TX/RX enable, Full Duplex, 10M speed.
25. [Conditional ETH_100M && HALF_DUPLEX] write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xC003) \u2014 TX/RX enable, Half Duplex, 100M speed.
26. [Conditional ETH_100M && !HALF_DUPLEX] write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0xE003) \u2014 TX/RX enable, Full Duplex, 100M speed.
27. [Conditional default && HALF_DUPLEX] write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x0003) \u2014 TX/RX enable, Half Duplex, default speed.
28. [Conditional default && !HALF_DUPLEX] write_enet_reg(mizar_ETHERNET0_MAC_CONFIGURATION, 0x2003) \u2014 TX/RX enable, Full Duplex, default speed.
29. write_reg(0xA0243ffc, enet_sel) \u2014 ETH VIP sequencer start.
30. write_reg(0xA0243ff8, 0xdeadbeef) \u2014 ETH VIP sequencer start.
31. write_enet_reg(mizar_ETHERNET0_MAC_EXT_CONFIGURATION, 0x0).
32. write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_OPERATION_MODE, 0x000f000a) \u2014 4096 Bytes TX Queue Size, Store&Forward Enabled, TX Q3.
33. write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE, 0x000f000a) \u2014 4096 Bytes TX Queue Size, Store&Forward Enabled, TX Q2.
34. write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE, 0x000f000a) \u2014 4096 Bytes TX Queue Size, Store&Forward Enabled, TX Q1.
35. write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE, 0x000f000a) \u2014 4096 Bytes TX Queue Size, Store&Forward Enabled, TX Q0.
36. write_enet_reg(mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT, 0x00000005) \u2014 Quantum/Weight for WRR/WFQ/DWRR.
37. write_enet_reg(mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT, 0x00000005).
38. write_enet_reg(mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT, 0x00000005).
39. write_enet_reg(mizar_ETHERNET0_MTL_TXQ0_QUANTUM_WEIGHT, 0x00000005).
40. write_enet_reg(mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS, 0x01000100) \u2014 RX Queue Overflow and TX Queue Underflow Interrupt Enable.
41. write_enet_reg(mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS, 0x01000100).
42. write_enet_reg(mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS, 0x01000100).
43. write_enet_reg(mizar_ETHERNET0_MTL_Q0_INTERRUPT_CONTROL_STATUS, 0x01000100).
44. write_enet_reg(mizar_ETHERNET0_DMA_SYSBUS_MODE, 0x0103000e) \u2014 Outstanding request write=1, read=4, FB, AALE, Burst 16/8/4, max=8.
45. Call preload_descriptor(0xE6000000, 0xE6008000, 0x3c, 0x5).
46. Call preload_descriptor(0xE6000050, 0xE600812c, 0x5E8, 0x5).
47. write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_RING_LENGTH, 0x000000A) \u2014 TX Descriptor Ring Length.
48. write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_RING_LENGTH, 0x000000A).
49. write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_RING_LENGTH, 0x000000A).
50. write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH, 0x000000A).
51. write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS, 0x00300000) \u2014 Base address of first TX descriptor.
52. write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS, 0x00200000).
53. write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS, 0x00100000).
54. write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS, 0xE6000000).
55. write_enet_reg(mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER, 0x00309730).
56. write_enet_reg(mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER, 0x00209730).
57. write_enet_reg(mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER, 0x00109730).
58. write_enet_reg(mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER, 0xE6009EB4).
59. write_enet_reg(mizar_ETHERNET0_DMA_CH3_CONTROL, 0x0).
60. write_enet_reg(mizar_ETHERNET0_DMA_CH2_CONTROL, 0x0).
61. write_enet_reg(mizar_ETHERNET0_DMA_CH1_CONTROL, 0x0).
62. write_enet_reg(mizar_ETHERNET0_DMA_CH0_CONTROL, 0x00000000).
63. write_enet_reg(mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE, 0x000F0C7) \u2014 Enable DMA channel interrupts.
64. write_enet_reg(mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE, 0x000F0C7).
65. write_enet_reg(mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE, 0x000F0C7).
66. write_enet_reg(mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE, 0x000F0C7).
67. write_reg(0xE68C2058, power) \u2014 Enable HSS Autoreg Ethernet Interrupts.
68. write_enet_reg(mizar_ETHERNET0_DMA_CH0_TX_CONTROL, 0x00100007) \u2014 START TX on DMA channel 0.
69. Enter for loop: i = 0; i < trns_count; i++ (trns_count = 10 default, 6 for ETH_10M).
70. [Loop iteration i=0] Enter while(int_pend) loop.
71. [Loop iteration i=0] Call wait_on(10) inside while(int_pend) loop, polling until int_pend becomes 0 (cleared by ISR).
72. [Loop iteration i=0] Exit while loop when int_pend == 0.
73. [Loop iteration i=0] Set int_pend = 1.
74. [Loop iteration i=1] Enter while(int_pend) loop.
75. [Loop iteration i=1] Call wait_on(10) inside while(int_pend) loop.
76. [Loop iteration i=1] Exit while loop when int_pend == 0.
77. [Loop iteration i=1] Set int_pend = 1.
78. [Loop iteration i=2] Enter while(int_pend) loop.
79. [Loop iteration i=2] Call wait_on(10) inside while(int_pend) loop.
80. [Loop iteration i=2] Exit while loop when int_pend == 0.
81. [Loop iteration i=2] Set int_pend = 1.
82. [Loop iteration i=3] Enter while(int_pend) loop.
83. [Loop iteration i=3] Call wait_on(10) inside while(int_pend) loop.
84. [Loop iteration i=3] Exit while loop when int_pend == 0.
85. [Loop iteration i=3] Set int_pend = 1.
86. [Loop iteration i=4] Enter while(int_pend) loop.
87. [Loop iteration i=4] Call wait_on(10) inside while(int_pend) loop.
88. [Loop iteration i=4] Exit while loop when int_pend == 0.
89. [Loop iteration i=4] Set int_pend = 1.
90. [Loop iteration i=5] Enter while(int_pend) loop.
91. [Loop iteration i=5] Call wait_on(10) inside while(int_pend) loop.
92. [Loop iteration i=5] Exit while loop when int_pend == 0.
93. [Loop iteration i=5] Set int_pend = 1.
94. [Loop iteration i=6] Enter while(int_pend) loop.
95. [Loop iteration i=6] Call wait_on(10) inside while(int_pend) loop.
96. [Loop iteration i=6] Exit while loop when int_pend == 0.
97. [Loop iteration i=6] Set int_pend = 1.
98. [Loop iteration i=7] Enter while(int_pend) loop.
99. [Loop iteration i=7] Call wait_on(10) inside while(int_pend) loop.
100. [Loop iteration i=7] Exit while loop when int_pend == 0.
101. [Loop iteration i=7] Set int_pend = 1.
102. [Loop iteration i=8] Enter while(int_pend) loop.
103. [Loop iteration i=8] Call wait_on(10) inside while(int_pend) loop.
104. [Loop iteration i=8] Exit while loop when int_pend == 0.
105. [Loop iteration i=8] Set int_pend = 1.
106. [Loop iteration i=9] Enter while(int_pend) loop.
107. [Loop iteration i=9] Call wait_on(10) inside while(int_pend) loop.
108. [Loop iteration i=9] Exit while loop when int_pend == 0.
109. [Loop iteration i=9] Set int_pend = 1.
110. Exit for loop after trns_count iterations.
111. Call wait_on(2000) \u2014 final wait after all transfers.
112. Call finish(0) \u2014 test completion with pass.
--- Default_IRQHandler (ISR) flow ---
113. Set int_pend = 0.
114. Compute irq_no = 42 + enet_select.
115. Set enet_sel = enet_select.
116. rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS) \u2014 read DMA interrupt status.
117. Set wdata = 0xffffffff.
118. write_enet_reg(mizar_ETHERNET0_DMA_CH0_STATUS, wdata) \u2014 write 1 to clear all DMA CH0 status bits.
119. rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS) \u2014 read DMA interrupt status again after clearing.
120. write_reg(0xE68C2050, power) \u2014 write HSS autoreg.
121. [Conditional CLRIRQ_42] GIC_ClearIRQ(42).
122. [Else] GIC_ClearIRQ(irq_no) where irq_no = 42 + enet_select.""",
        "Meta Impacted Registers": "mizar_ETHERNET0_MAC_ADDRESS3_HIGH; mizar_ETHERNET0_MAC_ADDRESS2_HIGH; mizar_ETHERNET0_MAC_ADDRESS1_HIGH; mizar_ETHERNET0_MAC_ADDRESS0_HIGH; mizar_ETHERNET0_MAC_ADDRESS3_LOW; mizar_ETHERNET0_MAC_ADDRESS2_LOW; mizar_ETHERNET0_MAC_ADDRESS1_LOW; mizar_ETHERNET0_MAC_ADDRESS0_LOW; mizar_ETHERNET0_MAC_PACKET_FILTER; mizar_ETHERNET0_MAC_CONFIGURATION; mizar_ETHERNET0_MAC_EXT_CONFIGURATION; mizar_ETHERNET0_MTL_TXQ3_OPERATION_MODE; mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE; mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE; mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE; mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT; mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT; mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT; mizar_ETHERNET0_MTL_TXQ0_QUANTUM_WEIGHT; mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS; mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS; mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS; mizar_ETHERNET0_MTL_Q0_INTERRUPT_CONTROL_STATUS; mizar_ETHERNET0_DMA_SYSBUS_MODE; mizar_ETHERNET0_DMA_CH3_TXDESC_RING_LENGTH; mizar_ETHERNET0_DMA_CH2_TXDESC_RING_LENGTH; mizar_ETHERNET0_DMA_CH1_TXDESC_RING_LENGTH; mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH; mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS; mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS; mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS; mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS; mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER; mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER; mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER; mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER; mizar_ETHERNET0_DMA_CH3_CONTROL; mizar_ETHERNET0_DMA_CH2_CONTROL; mizar_ETHERNET0_DMA_CH1_CONTROL; mizar_ETHERNET0_DMA_CH0_CONTROL; mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE; mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE; mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE; mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE; mizar_ETHERNET0_DMA_CH0_TX_CONTROL; mizar_ETHERNET0_DMA_INTERRUPT_STATUS; mizar_ETHERNET0_DMA_CH0_STATUS; 0xA0243ffc; 0xA0243ff8; 0xE68C2058; 0xE68C2050",
        "Meta Validation / Acceptance Criteria": """1. The test relies on interrupt-driven validation: for each of the trns_count transfer iterations (default 10, or 6 for ETH_10M), the test polls int_pend inside a while(int_pend) loop calling wait_on(10). The ISR Default_IRQHandler sets int_pend = 0 to signal transfer completion.
2. In the ISR, rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS) is read to check DMA interrupt status.
3. write_enet_reg(mizar_ETHERNET0_DMA_CH0_STATUS, 0xffffffff) clears all DMA CH0 status bits by writing 1 to clear.
4. rdata = read_enet_reg(mizar_ETHERNET0_DMA_INTERRUPT_STATUS) is read again after clearing to verify status was cleared.
5. GIC IRQ is cleared via GIC_ClearIRQ(42) (if CLRIRQ_42 defined) or GIC_ClearIRQ(irq_no) where irq_no = 42 + enet_select.
6. After all trns_count interrupt-driven transfer completions, wait_on(2000) provides a final settling delay.
7. finish(0) is called indicating test pass (argument 0 indicates success).
8. If any interrupt does not arrive, the while(int_pend) loop with wait_on(10) will continue polling indefinitely (no explicit timeout/fail path in source).
9. Global counters def_fail_cnt and wr_fail_cnt are initialized to 0 but not explicitly checked in this testcase source (may be used by external framework).""",
        "Meta Headers": "<stdio.h>; <stdlib.h>; <math.h>; <test_common.h>; <ethernet0/ethernet0_def.h>; <ethernet0/ethernet0_offset.h>; <ethernet0/ethernet0_funcs.h>; <ethernet0/ethernet0_programming_sequence.h>",
        "Meta Macros": """#define (conditional) ETH_10M \u2014 used to select 10M speed path and set trns_count = 6
#define (conditional) ETH_100M \u2014 used to select 100M speed path
#define (conditional) HALF_DUPLEX \u2014 used to select half-duplex MAC configuration
#define (conditional) SEL_ENET0 \u2014 used to select enet_intf_sel(0)
#define (conditional) SEL_ENET1 \u2014 used to select enet_intf_sel(1)
#define (conditional) SEL_ENET2 \u2014 used to select enet_intf_sel(2)
#define (conditional) SEL_ENET3 \u2014 used to select enet_intf_sel(3)
#define (conditional) DEBUG_DISPLAY \u2014 used to enable printf debug output
#define (conditional) CLRIRQ_42 \u2014 used to select GIC_ClearIRQ(42) instead of GIC_ClearIRQ(irq_no)
Note: The following are external macro identifiers used in register-access calls (definitions expected in ethernet0_offset.h or ethernet0_def.h):
mizar_ETHERNET0_MAC_ADDRESS3_HIGH
mizar_ETHERNET0_MAC_ADDRESS2_HIGH
mizar_ETHERNET0_MAC_ADDRESS1_HIGH
mizar_ETHERNET0_MAC_ADDRESS0_HIGH
mizar_ETHERNET0_MAC_ADDRESS3_LOW
mizar_ETHERNET0_MAC_ADDRESS2_LOW
mizar_ETHERNET0_MAC_ADDRESS1_LOW
mizar_ETHERNET0_MAC_ADDRESS0_LOW
mizar_ETHERNET0_MAC_PACKET_FILTER
mizar_ETHERNET0_MAC_CONFIGURATION
mizar_ETHERNET0_MAC_EXT_CONFIGURATION
mizar_ETHERNET0_MTL_TXQ3_OPERATION_MODE
mizar_ETHERNET0_MTL_TXQ2_OPERATION_MODE
mizar_ETHERNET0_MTL_TXQ1_OPERATION_MODE
mizar_ETHERNET0_MTL_TXQ0_OPERATION_MODE
mizar_ETHERNET0_MTL_TXQ3_QUANTUM_WEIGHT
mizar_ETHERNET0_MTL_TXQ2_QUANTUM_WEIGHT
mizar_ETHERNET0_MTL_TXQ1_QUANTUM_WEIGHT
mizar_ETHERNET0_MTL_TXQ0_QUANTUM_WEIGHT
mizar_ETHERNET0_MTL_Q3_INTERRUPT_CONTROL_STATUS
mizar_ETHERNET0_MTL_Q2_INTERRUPT_CONTROL_STATUS
mizar_ETHERNET0_MTL_Q1_INTERRUPT_CONTROL_STATUS
mizar_ETHERNET0_MTL_Q0_INTERRUPT_CONTROL_STATUS
mizar_ETHERNET0_DMA_SYSBUS_MODE
mizar_ETHERNET0_DMA_CH3_TXDESC_RING_LENGTH
mizar_ETHERNET0_DMA_CH2_TXDESC_RING_LENGTH
mizar_ETHERNET0_DMA_CH1_TXDESC_RING_LENGTH
mizar_ETHERNET0_DMA_CH0_TXDESC_RING_LENGTH
mizar_ETHERNET0_DMA_CH3_TXDESC_LIST_ADDRESS
mizar_ETHERNET0_DMA_CH2_TXDESC_LIST_ADDRESS
mizar_ETHERNET0_DMA_CH1_TXDESC_LIST_ADDRESS
mizar_ETHERNET0_DMA_CH0_TXDESC_LIST_ADDRESS
mizar_ETHERNET0_DMA_CH3_TXDESC_TAIL_POINTER
mizar_ETHERNET0_DMA_CH2_TXDESC_TAIL_POINTER
mizar_ETHERNET0_DMA_CH1_TXDESC_TAIL_POINTER
mizar_ETHERNET0_DMA_CH0_TXDESC_TAIL_POINTER
mizar_ETHERNET0_DMA_CH3_CONTROL
mizar_ETHERNET0_DMA_CH2_CONTROL
mizar_ETHERNET0_DMA_CH1_CONTROL
mizar_ETHERNET0_DMA_CH0_CONTROL
mizar_ETHERNET0_DMA_CH1_INTERRUPT_ENABLE
mizar_ETHERNET0_DMA_CH2_INTERRUPT_ENABLE
mizar_ETHERNET0_DMA_CH3_INTERRUPT_ENABLE
mizar_ETHERNET0_DMA_CH0_INTERRUPT_ENABLE
mizar_ETHERNET0_DMA_CH0_TX_CONTROL
mizar_ETHERNET0_DMA_INTERRUPT_STATUS
mizar_ETHERNET0_DMA_CH0_STATUS""",
        "Meta Arrays": "NA"
    }
]

# ============================================================
# TESTPLAN SHEET COLUMNS
# ============================================================
TESTPLAN_COLUMNS = [
    "Index",
    "SS / Module",
    "Feature",
    "Test Case Name",
    "Test Description",
    "Speed",
    "Mode",
    "Memory Start Offset",
    "Memory End Offset",
    "Remarks",
    "Test Steps / Procedure",
    "Impacted Registers",
    "Validation / Acceptance Criteria",
    "Code Generation"
]

# ============================================================
# METADATA SHEET COLUMNS
# ============================================================
METADATA_COLUMNS = [
    "Index",
    "Test Case Name",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays"
]

def create_workbook():
    """Create and save the Excel workbook."""
    # IST timestamp
    ist = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(ist)
    timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
    filename = f"Ethernet_TestPlan_{timestamp}.xlsx"
    
    print(f"Generating workbook: {filename}")
    print(f"IST Timestamp: {now_ist.strftime('%Y-%m-%d %H:%M:%S IST')}")
    
    # Create workbook
    wb = Workbook()
    
    # ============================================================
    # TESTPLAN SHEET
    # ============================================================
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    
    # Header formatting
    header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
    header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    header_alignment = Alignment(horizontal='center', vertical='center', wrap_text=True)
    cell_alignment = Alignment(vertical='top', wrap_text=True)
    thin_border = Border(
        left=Side(style='thin'),
        right=Side(style='thin'),
        top=Side(style='thin'),
        bottom=Side(style='thin')
    )
    
    # Write TestPlan headers
    for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_alignment
        cell.border = thin_border
    
    # Write TestPlan data rows
    for row_idx, row_data in enumerate(TEST_DATA, 2):
        for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
            value = row_data.get(col_name, "")
            if value is None:
                value = ""
            cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = cell_alignment
            cell.border = thin_border
    
    # Freeze first row
    ws_tp.freeze_panes = 'A2'
    
    # Auto-size columns with max width
    for col_idx, col_name in enumerate(TESTPLAN_COLUMNS, 1):
        max_length = len(col_name)
        for row in ws_tp.iter_rows(min_row=2, max_row=ws_tp.max_row, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        if len(line) > max_length:
                            max_length = len(line)
        adjusted_width = min(max_length + 4, 60)
        if adjusted_width < 12:
            adjusted_width = 12
        ws_tp.column_dimensions[ws_tp.cell(row=1, column=col_idx).column_letter].width = adjusted_width
    
    # ============================================================
    # METADATA SHEET
    # ============================================================
    ws_md = wb.create_sheet(title="MetaData")
    
    # Write MetaData headers
    for col_idx, col_name in enumerate(METADATA_COLUMNS, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = header_alignment
        cell.border = thin_border
    
    # Write MetaData data rows
    for row_idx, row_data in enumerate(TEST_DATA, 2):
        for col_idx, col_name in enumerate(METADATA_COLUMNS, 1):
            value = row_data.get(col_name, "")
            if value is None:
                value = ""
            cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = cell_alignment
            cell.border = thin_border
    
    # Freeze first row
    ws_md.freeze_panes = 'A2'
    
    # Auto-size columns with max width
    for col_idx, col_name in enumerate(METADATA_COLUMNS, 1):
        max_length = len(col_name)
        for row in ws_md.iter_rows(min_row=2, max_row=ws_md.max_row, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        if len(line) > max_length:
                            max_length = len(line)
        adjusted_width = min(max_length + 4, 60)
        if adjusted_width < 12:
            adjusted_width = 12
        ws_md.column_dimensions[ws_md.cell(row=1, column=col_idx).column_letter].width = adjusted_width
    
    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = 'veryHidden'
    
    # ============================================================
    # SAVE WORKBOOK
    # ============================================================
    wb.save(filename)
    print(f"Workbook saved: {filename}")
    
    # ============================================================
    # POST-SAVE VALIDATION
    # ============================================================
    # Check file exists and size
    if not os.path.exists(filename):
        print("VALIDATION FAILED: File does not exist")
        return None, "FAILED"
    
    file_size = os.path.getsize(filename)
    if file_size == 0:
        print("VALIDATION FAILED: File size is 0")
        return None, "FAILED"
    
    print(f"File size: {file_size} bytes")
    
    # Reopen and validate
    wb_check = load_workbook(filename)
    sheet_names = wb_check.sheetnames
    print(f"Sheet names: {sheet_names}")
    
    if "TestPlan" not in sheet_names:
        print("VALIDATION FAILED: TestPlan sheet missing")
        return None, "FAILED"
    
    if "MetaData" not in sheet_names:
        print("VALIDATION FAILED: MetaData sheet missing")
        return None, "FAILED"
    
    # Validate MetaData content
    ws_md_check = wb_check["MetaData"]
    meta_fields_to_validate = [
        "Meta Test Description",
        "Meta Test Steps / Procedure",
        "Meta Impacted Registers",
        "Meta Validation / Acceptance Criteria",
        "Meta Headers",
        "Meta Macros",
        "Meta Arrays"
    ]
    
    # Get column mapping from header row
    col_map = {}
    for col_idx in range(1, ws_md_check.max_column + 1):
        header_val = ws_md_check.cell(row=1, column=col_idx).value
        if header_val:
            col_map[header_val] = col_idx
    
    validation_passed = True
    for field in meta_fields_to_validate:
        if field not in col_map:
            print(f"VALIDATION FAILED: Column '{field}' not found in MetaData")
            validation_passed = False
            continue
        
        cell_value = ws_md_check.cell(row=2, column=col_map[field]).value
        expected_value = TEST_DATA[0].get(field, "")
        
        if cell_value != expected_value:
            print(f"VALIDATION FAILED: '{field}' content mismatch")
            print(f"  Expected length: {len(str(expected_value))}")
            print(f"  Got length: {len(str(cell_value)) if cell_value else 0}")
            validation_passed = False
        else:
            print(f"VALIDATION PASSED: '{field}' matches exactly")
    
    # Validate TestPlan row count
    ws_tp_check = wb_check["TestPlan"]
    tp_rows = ws_tp_check.max_row - 1  # minus header
    print(f"TestPlan data rows: {tp_rows}")
    
    md_rows = ws_md_check.max_row - 1  # minus header
    print(f"MetaData data rows: {md_rows}")
    
    wb_check.close()
    
    if not validation_passed:
        print("OVERALL VALIDATION: FAILED")
        return filename, "FAILED"
    
    print("OVERALL VALIDATION: PASSED")
    return filename, "PASSED"


if __name__ == "__main__":
    filename, validation = create_workbook()
    if filename and validation == "PASSED":
        print(f"\nSUCCESS: {filename}")
        print(f"Validation: {validation}")
    else:
        print(f"\nFAILURE")
        print(f"Validation: {validation}")
        sys.exit(1)
