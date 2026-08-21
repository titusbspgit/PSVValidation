#!/usr/bin/env python3
"""PCIE TestPlan XLSX Generator - Generates genuine Office Open XML workbook."""
import json
import os
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
except ImportError:
    import subprocess
    subprocess.check_call(['pip', 'install', 'openpyxl'])
    from openpyxl import Workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side

# IST timestamp
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'PCIE_TestPlan_{timestamp}.xlsx'

# Output directory
output_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
os.makedirs(output_dir, exist_ok=True)
output_path = os.path.join(output_dir, filename)

# Testcase data
testcases = [
    {
        "Index": 1,
        "SS / Module": "PCIE",
        "Test Case Name": "pcie_device_enumerate_test",
        "Feature": "Device Enumeration and Link Training",
        "Test Description": "This test validates PCIe device enumeration by performing link training on both dual-mode controllers (DM0 and DM1) in x4 configuration, programming the AXI cache coherency control register for both PCIE0 and PCIE1 instances, polling the SII link status registers until link-up is confirmed, reading the Vendor ID from the endpoint configuration space, programming memory base address registers, performing BAR sizing and assignment on both slave ports, and finally disabling cache programming before completing the test via a synchronization handshake register poll.",
        "Test Type": "Functional",
        "Test Steps / Procedure": "1. Write to the synchronization handshake register to initialize the test.\n2. Initiate PCIe link training in x4 mode on both DM0 and DM1 controllers based on the configured role (RC or EP).\n3. Program the COHERENCY_CONTROL_3_OFF register for PCIE0 instance by performing read-modify-write to set CFG_MSTR_AWCACHE_MODE (bits 11-14) and CFG_MSTR_ARCACHE_MODE (bits 3-6) fields to 0xF.\n4. Program the COHERENCY_CONTROL_3_OFF register for PCIE0 instance by performing read-modify-write to set CFG_MSTR_AWCACHE_VALUE (bits 27-30) and CFG_MSTR_ARCACHE_VALUE (bits 19-22) fields to 0xF.\n5. Repeat steps 3-4 for the PCIE1 instance of COHERENCY_CONTROL_3_OFF.\n6. Wait for 20 cycles, then re-program all four cache mode and cache value fields in a single read-modify-write pass for both PCIE0 and PCIE1.\n7. Poll the SII0 link status register at offset 0xC0 until link-up status bits (mask 0xD1) are all set.\n8. Poll the SII1 link status register at offset 0xC0 until link-up status bits (mask 0xD1) are all set.\n9. Read the Vendor ID from the PCIE slave 0 configuration space at offset 0x0 and print the result.\n10. Write command register value 0x7 to PCIE slave 0 at offset 0x4 to enable memory space, I/O space, and bus master.\n11. Program memory base addresses for DM0 and DM1 in x4 mode.\n12. Write to system-level control registers to configure the PCIe subsystem.\n13. Disable cache programming by performing read-modify-write on COHERENCY_CONTROL_3_OFF for both PCIE0 and PCIE1, clearing CFG_MSTR_ARCACHE_VALUE (bits 19-22) and CFG_MSTR_AWCACHE_VALUE (bits 27-30) to 0x0.\n14. Perform BAR sizing on PCIE slave 1 by writing 0xFFFFFFFF to BAR registers (offsets 0x10-0x24), reading back to determine BAR sizes, then programming final BAR values.\n15. Repeat BAR sizing and assignment on PCIE slave 0.\n16. Poll the synchronization handshake register until the expected completion value 0x12345678 is read, confirming test completion.\n17. Call finish to end the test with pass status.",
        "Impacted Registers": "COHERENCY_CONTROL_3_OFF",
        "Validation / Acceptance Criteria": "1. PCIe link training completes successfully on both DM0 and DM1 controllers.\n2. SII0 and SII1 link status registers report link-up (bits matching mask 0xD1).\n3. Vendor ID is successfully read from the endpoint configuration space.\n4. COHERENCY_CONTROL_3_OFF register read-modify-write operations complete without error for both PCIE0 and PCIE1 instances.\n5. BAR sizing returns valid BAR size information for all BARs on both slave ports.\n6. Synchronization handshake register returns 0x12345678 indicating successful end-to-end enumeration.",
        "Remarks": "Test exercises both PCIE0 and PCIE1 controller instances. Link training mode (RC or EP) is selected via compile-time defines (DM0_RC, DM1_RC, DM0_EP, DM1_EP). The test includes both cache enable and cache disable phases for the coherency control register. BAR sizing is performed on both slave port 0 and slave port 1.",
        "Meta Test Description": "This testcase performs PCIe device enumeration. It begins by writing 0x0 to 0xE6004100 (synchronization register). Link training is initiated via link_training_dm0_x4(4) and link_training_dm1_x4(4) based on compile-time defines (DM0_RC, DM1_RC, DM0_EP, DM1_EP). Cache programming is performed by read-modify-write on mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF and mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF using set_data() to set bits [11:14]=0xF (CFG_MSTR_AWCACHE_MODE), bits [3:6]=0xF (CFG_MSTR_ARCACHE_MODE), bits [27:30]=0xF (CFG_MSTR_AWCACHE_VALUE), bits [19:22]=0xF (CFG_MSTR_ARCACHE_VALUE). After wait_on(20), the same fields are re-programmed in a combined pass. SII0 register at offset 0xC0 is polled until (data_rd & 0xD1) == 0xD1. SII1 register at offset 0xC0 is polled similarly. non_secure_prot_nic() is called. Vendor ID is read via read_pcie_slv0_reg(0x0). Command register is written via write_pcie_slv0_reg(0x4, 0x7). mem_base_program_dm0_x4() and mem_base_program_dm1_x4() are called. System registers 0xE690000C, 0xE6900010, 0xE6900014, 0xE6900018, 0xE6900030, 0xE6900034 are written with 0x1. Cache disable phase sets CFG_MSTR_ARCACHE_VALUE (bits 19-22) to 0x0 and CFG_MSTR_AWCACHE_VALUE (bits 27-30) to 0x0 via read-modify-write on both PCIE0 and PCIE1 COHERENCY_CONTROL_3_OFF. BAR sizing is performed on pcie_slv1 (offsets 0x10-0x24) by writing 0xFFFFFFFF, reading back, then writing final values. Same BAR sizing on pcie_slv0. Finally, 0xE6004100 is polled until data_rd == 0x12345678, then finish(0) is called.",
        "Meta Test Steps / Procedure": "1. write_reg(0xE6004100, 0x0) - initialize synchronization register.\n2. Conditional link training: link_training_dm0_x4(4) if DM0_RC or DM0_EP defined; link_training_dm1_x4(4) if DM1_RC or DM1_EP defined.\n3. CACHE PROGRAMMING phase: rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 11, 14, 0xF); rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xF); write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1); Repeat for PCIE1.\n4. wait_on(20)\n5. Combined cache programming pass for PCIE0 and PCIE1.\n6-8. Poll SII0/SII1 at 0xC0 until link-up.\n9-10. Read Vendor ID, write command register.\n11. Program memory base addresses.\n12-13. Write system registers, cache disable phase.\n14-15. BAR sizing on pcie_slv1 and pcie_slv0.\n16. Poll 0xE6004100 until 0x12345678.\n17. finish(0).",
        "Meta Impacted Registers": "0xE6004100; mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF; mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF; 0xE690000C; 0xE6900010; 0xE6900014; 0xE6900018; 0xE6900030; 0xE6900034"
    },
    {
        "Index": 2,
        "SS / Module": "PCIE",
        "Test Case Name": "pcie_dma_write_test",
        "Feature": "DMA Write and Read Data Transfer",
        "Test Description": "This test validates PCIe DMA write and read data transfer operations across all four DMA channels (Channel 0 through Channel 3) on both dual-mode controllers (DM0 and DM1). The test performs link training in x4 mode, waits for link-up confirmation by polling the SII link status register, reads the Vendor ID from the endpoint, programs BAR and memory base addresses, preloads source memory with known data patterns, configures the GIC for interrupt-based DMA completion, unmasks DMA write and read interrupts via the DMA_WRITE_INT_MASK_OFF and DMA_READ_INT_MASK_OFF registers, sequentially programs and triggers DMA write transfers on all four channels using the DMA_WRITE_DOORBELL_OFF register, waits for each transfer to complete via interrupt, then performs DMA read-back transfers on all four channels using the DMA_READ_DOORBELL_OFF register.",
        "Test Type": "Functional",
        "Test Steps / Procedure": "1. Initialize the synchronization handshake register.\n2. Initiate PCIe link training in x4 mode.\n3. Poll the SII link status register until link-up is confirmed (mask 0xD1).\n4. Read the Vendor ID from the endpoint configuration space.\n5. Write the command register to enable memory space, I/O space, and bus master.\n6. Program BAR registers and memory base addresses.\n7. Poll the synchronization handshake register until 0x12345678.\n8. Set DMA transfer length to 0x40 bytes.\n9. Preload source memory with 0xC0DEBEED and 0xF00DDEAF patterns.\n10. Initialize GIC and enable all IRQs.\n11. Unmask DMA interrupts by writing 0x0 to DMA_WRITE_INT_MASK_OFF and DMA_READ_INT_MASK_OFF.\n12-13. Program and trigger DMA write channels 0-3 via DMA_WRITE_DOORBELL_OFF.\n14-15. Program and trigger DMA read channels 0-3 via DMA_READ_DOORBELL_OFF.\n16. Verify each DMA transfer completes via interrupt handler.\n17. Call finish(0).",
        "Impacted Registers": "DMA_WRITE_INT_MASK_OFF; DMA_READ_INT_MASK_OFF; DMA_WRITE_DOORBELL_OFF; DMA_READ_DOORBELL_OFF; DMA_WRITE_INT_STATUS_OFF; DMA_READ_INT_STATUS_OFF; DMA_WRITE_INT_CLEAR_OFF; DMA_READ_INT_CLEAR_OFF",
        "Validation / Acceptance Criteria": "1. PCIe link training completes successfully.\n2. Vendor ID is successfully read.\n3. DMA interrupt masks written with 0x0.\n4. Each DMA write channel (0-3) completes and triggers interrupt.\n5. Each DMA read channel (0-3) completes and triggers interrupt.\n6. All DMA interrupts cleared via INT_CLEAR registers.\n7. GIC interrupts cleared after each service.\n8. Test completes with finish(0).",
        "Remarks": "Test exercises both PCIE0 and PCIE1 via compile-time defines. All four DMA write and read channels tested sequentially. Interrupt-driven completion with polling wait loop. Source data preloaded with two distinct patterns.",
        "Meta Test Description": "This testcase performs PCIe DMA write and read data transfer operations. It begins by writing 0x0 to 0xE6004100 (synchronization register). Link training is initiated. For DM0_RC: SII0 register at offset 0xC0 is polled until link-up. Vendor ID read, command register written, BAR and memory base programmed. For DM1_RC: similar on SII1 and pcie_slv1. Transfer length 0x40, source at 0xE6000000. Source preloaded: 128 words with 0xC0DEBEED, 128 words with 0xF00DDEAF. GIC configured. DMA interrupts unmasked. Channels 0-3 write triggered via DMA_WRITE_DOORBELL_OFF. Channels 0-3 read triggered via DMA_READ_DOORBELL_OFF. IRQ Handler reads status, masks with 0x0000000F, clears interrupts.",
        "Meta Test Steps / Procedure": "1. write_reg(0xE6004100, 0x0).\n2. Link training.\n3-6. Link-up poll and enumeration for DM0_RC/DM1_RC.\n7. non_secure_prot_nic().\n8. Poll 0xE6004100 until 0x12345678.\n9. Configure addresses and preload data.\n10-11. GIC setup and DMA interrupt unmask.\n12-20. DM0_RC Channel 0-3 write and read operations.\n21-29. DM1_RC Channel 0-3 write and read operations.\n30. finish(0).\n31-33. IRQ Handler for DM0_RC and DM1_RC.",
        "Meta Impacted Registers": "0xE6004100; mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF; mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF; mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF; mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF; mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF; mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF; mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF; mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF; mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF; mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF; mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF; mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF; mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF; mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF; mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF; mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF"
    },
    {
        "Index": 3,
        "SS / Module": "PCIE",
        "Test Case Name": "pcie_mem_wr_rd_test",
        "Feature": "Memory Write and Read Verification",
        "Test Description": "This test validates PCIe memory write and read operations through the slave ports on both dual-mode controllers (DM0 and DM1). The test performs link training in x4 mode, programs the AXI cache coherency control register (COHERENCY_CONTROL_3_OFF) for both PCIE0 and PCIE1 instances, polls the SII link status registers until link-up is confirmed, reads the Vendor ID, programs BAR registers and memory base addresses, signals readiness via synchronization handshake register, then disables cache programming. The test performs memory write-read verification at multiple addresses and polls for completion.",
        "Test Type": "Functional",
        "Test Steps / Procedure": "1. Initialize the synchronization handshake register.\n2. Initiate PCIe link training in x4 mode.\n3. Enable AXI cache coherency by programming COHERENCY_CONTROL_3_OFF.\n4. Program cache value fields.\n5. Wait 20 cycles, re-program in combined pass.\n6. Poll SII link status until link-up.\n7. Read Vendor ID.\n8. Write command register 0x7.\n9. Program BAR and memory base addresses.\n10. Call non_secure_prot_nic().\n11. Write 0x11111111 to synchronization register.\n12. Disable cache programming.\n13-14. Wait and combined disable pass.\n15. Perform memory write-read verification.\n16. Poll synchronization register until 0x12345678.\n17. Call finish(0).",
        "Impacted Registers": "COHERENCY_CONTROL_3_OFF",
        "Validation / Acceptance Criteria": "1. PCIe link training completes successfully.\n2. SII link status reports link-up.\n3. Vendor ID successfully read.\n4. COHERENCY_CONTROL_3_OFF operations complete without error.\n5. Memory write-read operations complete successfully.\n6. Synchronization register returns 0x12345678.\n7. Test completes with finish(0).",
        "Remarks": "Test exercises both PCIE0 and PCIE1 controller instances. Includes cache enable and disable phases. For RC mode, memory write-read at three addresses. For EP mode, at five BAR1 addresses. Synchronization register written with 0x11111111 after setup.",
        "Meta Test Description": "This testcase performs PCIe memory write and read verification. It begins by writing 0x0 to 0xE6004100. Link training initiated. Cache programming performed by read-modify-write on COHERENCY_CONTROL_3_OFF for both PCIE0 and PCIE1. SII polled until link-up. Enumeration performed. write_reg(0xE6004100, 0x11111111) signals readiness. Cache disable phase clears value fields. Memory write-read: DM0_RC at 0x01040000/0xa5a5a5a5, 0x01000020/0xa6a6a6a6, 0x01004000/0xa7a7a7a7. DM1_RC at 0x01040000/0xb5b5b5b5, etc. DM0_EP/DM1_EP at BAR1 addresses with 0x5a5a5a5a. Poll 0xE6004100 until 0x12345678. finish(0).",
        "Meta Test Steps / Procedure": "1. write_reg(0xE6004100, 0x0).\n2. Link training.\n3-6. CACHE PROGRAMMING phase.\n7-9. SII link-up poll.\n10-13. Enumeration for all modes.\n14. non_secure_prot_nic().\n15. write_reg(0xE6004100, 0x11111111).\n16-19. DISABLE_CACHE PROGRAMMING phase.\n20. wait_on(30).\n21-24. Memory write-read operations.\n25-26. Poll and finish(0).",
        "Meta Impacted Registers": "0xE6004100; mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF; mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF"
    },
    {
        "Index": 4,
        "SS / Module": "PCIE",
        "Test Case Name": "pcie_reg_wr_rd_test",
        "Feature": "Register Write-Read and Reset Value Verification",
        "Test Description": "This test validates register reset values and write-read accessibility across multiple register groups on both PCIE0 and PCIE1 controller instances. Phase 1: Reset Value Check reads five DBI registers, three SII registers, and three PHY registers comparing against defaults. Phase 2: Write-Read Check iterates through three test patterns writing and reading back all register groups with appropriate masks.",
        "Test Type": "Register_RW",
        "Test Steps / Procedure": "1. Enter test case and invoke reset value check.\n2-3. Read RC0 and RC1 DBI registers, compare against 0x0.\n4-5. Read SII0 and SII1 registers, compare against 0x0.\n6. Assert PHY reset by writing 0x01203000.\n7-8. Read PHY0 and PHY1 registers with 16-bit extraction.\n9. Enter write-read check phase.\n10. For each pattern (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555), write to all registers.\n11. Write masked patterns to SII registers.\n12. Write PHY patterns (0x7BAF, 0x1, 0x003B) masked with 0x1FFF.\n13-18. Read back and verify all register groups.\n19. Repeat for all patterns.\n20. Call finish with combined error count.",
        "Impacted Registers": "MSI_CAP_OFF_08H_REG; MSI_CAP_OFF_10H_REG; FILTER_MASK_2_OFF; AXI_MSTR_MSG_ADDR_HIGH_OFF; UTILITY_OFF",
        "Validation / Acceptance Criteria": "1-6. All register groups read back expected defaults during reset check.\n7. For each test pattern, DBI registers read back exact written value.\n8. SII registers read back masked values.\n9. PHY registers read back masked values after 16-bit extraction.\n10. Test completes with finish(0) indicating zero errors.",
        "Remarks": "Test covers six register groups across two PCIE instances. SII registers use per-register write masks. PHY registers use 13-bit write mask (0x1FFF) and require 16-bit extraction. PHY reset asserted before PHY access. Three write-read patterns for DBI/SII; three PHY-specific patterns.",
        "Meta Test Description": "This testcase performs register write-read and reset value verification. Global arrays define register addresses: rc0_ctl_addr[5] for PCIE0 DBI registers, rc1_ctl_addr[5] for PCIE1, sii0_addr[3] and sii1_addr[3] for SII, phy0_addr[3]={0xE68860B8, 0xE68862B8, 0xE68864B8}, phy1_addr[3]={0xE68A60B8, 0xE68A62B8, 0xE68A64B8}. test_case() calls chk_rst_val() then chk_rd_wr(), then finish(err2||err1). chk_rst_val() reads all registers and compares against defaults. chk_rd_wr() uses chk_val[6] and chk_val_phy[3]={0x7BAF, 0x1, 0x003B}. Iterates j=0..2 writing and reading back.",
        "Meta Test Steps / Procedure": "1. Global initialization: err1=0, err2=0.\n2. test_case() entry.\n3. Call chk_rst_val().\n4-10. RESET VALUE CHECK for all register groups.\n11. Call chk_rd_wr().\n12-26. WRITE-READ CHECK outer loop j=0..2.\n27. finish(err2 || err1).",
        "Meta Impacted Registers": "mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG; mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG; mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF; mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF; mizar_PCIE0_DBI_DSP_UTILITY_OFF; mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG; mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG; mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF; mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF; mizar_PCIE1_DBI_DSP_UTILITY_OFF; mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2; mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3; mizar_PCIE0_SII_PHY_CONTROL_23; mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2; mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3; mizar_PCIE1_SII_PHY_CONTROL_23; 0xE68860B8; 0xE68862B8; 0xE68864B8; 0xE68A60B8; 0xE68A62B8; 0xE68A64B8; mizar_PCIE0_SII_PHY_RST_CONTROL; mizar_PCIE1_SII_PHY_RST_CONTROL"
    }
]

# Register details data
register_details = [
    # TC1: COHERENCY_CONTROL_3_OFF fields
    ["pcie_device_enumerate_test", "COHERENCY_CONTROL_3_OFF", "PORT_LOGIC", "0x700", "0x1E8", "0x8E8", "Reserved_0_2", 0, 3, "dc", "NA"],
    ["pcie_device_enumerate_test", "COHERENCY_CONTROL_3_OFF", "PORT_LOGIC", "0x700", "0x1E8", "0x8E8", "CFG_MSTR_ARCACHE_MODE", 3, 4, "rw", "0x0"],
    ["pcie_device_enumerate_test", "COHERENCY_CONTROL_3_OFF", "PORT_LOGIC", "0x700", "0x1E8", "0x8E8", "Reserved_7_10", 7, 4, "dc", "NA"],
    ["pcie_device_enumerate_test", "COHERENCY_CONTROL_3_OFF", "PORT_LOGIC", "0x700", "0x1E8", "0x8E8", "CFG_MSTR_AWCACHE_MODE", 11, 4, "rw", "0x0"],
    ["pcie_device_enumerate_test", "COHERENCY_CONTROL_3_OFF", "PORT_LOGIC", "0x700", "0x1E8", "0x8E8", "Reserved_15_18", 15, 4, "dc", "NA"],
    ["pcie_device_enumerate_test", "COHERENCY_CONTROL_3_OFF", "PORT_LOGIC", "0x700", "0x1E8", "0x8E8", "CFG_MSTR_ARCACHE_VALUE", 19, 4, "rw", "0x0"],
    ["pcie_device_enumerate_test", "COHERENCY_CONTROL_3_OFF", "PORT_LOGIC", "0x700", "0x1E8", "0x8E8", "Reserved_23_26", 23, 4, "dc", "NA"],
    ["pcie_device_enumerate_test", "COHERENCY_CONTROL_3_OFF", "PORT_LOGIC", "0x700", "0x1E8", "0x8E8", "CFG_MSTR_AWCACHE_VALUE", 27, 4, "rw", "0x0"],
    ["pcie_device_enumerate_test", "COHERENCY_CONTROL_3_OFF", "PORT_LOGIC", "0x700", "0x1E8", "0x8E8", "Reserved_31_31", 31, 1, "dc", "NA"],
    ["pcie_device_enumerate_test", "NA", "NA", "NA", "NA", "0xE6004100", "NA", "NA", "NA", "NA", "NA"],
    # TC2: DMA registers
    ["pcie_dma_write_test", "DMA_WRITE_DOORBELL_OFF", "DMA_CAP", "0x380000", "0x10", "0x380010", "WR_DOORBELL_NUM", 0, 3, "other", "0x0"],
    ["pcie_dma_write_test", "DMA_WRITE_DOORBELL_OFF", "DMA_CAP", "0x380000", "0x10", "0x380010", "RSVDP_3", 3, 28, "ro", "0x0"],
    ["pcie_dma_write_test", "DMA_WRITE_DOORBELL_OFF", "DMA_CAP", "0x380000", "0x10", "0x380010", "WR_STOP", 31, 1, "other", "0x0"],
    ["pcie_dma_write_test", "DMA_READ_DOORBELL_OFF", "DMA_CAP", "0x380000", "0x30", "0x380030", "RD_DOORBELL_NUM", 0, 3, "other", "0x0"],
    ["pcie_dma_write_test", "DMA_READ_DOORBELL_OFF", "DMA_CAP", "0x380000", "0x30", "0x380030", "RSVDP_3", 3, 28, "ro", "0x0"],
    ["pcie_dma_write_test", "DMA_READ_DOORBELL_OFF", "DMA_CAP", "0x380000", "0x30", "0x380030", "RD_STOP", 31, 1, "other", "0x0"],
    ["pcie_dma_write_test", "DMA_WRITE_INT_STATUS_OFF", "DMA_CAP", "0x380000", "0x4C", "0x38004C", "WR_DONE_INT_STATUS", 0, 8, "other", "0x0"],
    ["pcie_dma_write_test", "DMA_WRITE_INT_STATUS_OFF", "DMA_CAP", "0x380000", "0x4C", "0x38004C", "WR_ABORT_INT_STATUS", 16, 8, "other", "0x0"],
    ["pcie_dma_write_test", "DMA_WRITE_INT_MASK_OFF", "DMA_CAP", "0x380000", "0x54", "0x380054", "WR_DONE_INT_MASK", 0, 4, "rw", "0xF"],
    ["pcie_dma_write_test", "DMA_WRITE_INT_MASK_OFF", "DMA_CAP", "0x380000", "0x54", "0x380054", "WR_ABORT_INT_MASK", 16, 4, "rw", "0xF"],
    ["pcie_dma_write_test", "DMA_WRITE_INT_CLEAR_OFF", "DMA_CAP", "0x380000", "0x58", "0x380058", "WR_DONE_INT_CLEAR", 0, 4, "other", "0x0"],
    ["pcie_dma_write_test", "DMA_WRITE_INT_CLEAR_OFF", "DMA_CAP", "0x380000", "0x58", "0x380058", "WR_ABORT_INT_CLEAR", 16, 4, "other", "0x0"],
    ["pcie_dma_write_test", "DMA_READ_INT_STATUS_OFF", "DMA_CAP", "0x380000", "0xA0", "0x3800A0", "RD_DONE_INT_STATUS", 0, 8, "other", "0x0"],
    ["pcie_dma_write_test", "DMA_READ_INT_STATUS_OFF", "DMA_CAP", "0x380000", "0xA0", "0x3800A0", "RD_ABORT_INT_STATUS", 16, 8, "other", "0x0"],
    ["pcie_dma_write_test", "DMA_READ_INT_MASK_OFF", "DMA_CAP", "0x380000", "0xA8", "0x3800A8", "RD_DONE_INT_MASK", 0, 4, "rw", "0xF"],
    ["pcie_dma_write_test", "DMA_READ_INT_MASK_OFF", "DMA_CAP", "0x380000", "0xA8", "0x3800A8", "RD_ABORT_INT_MASK", 16, 4, "rw", "0xF"],
    ["pcie_dma_write_test", "DMA_READ_INT_CLEAR_OFF", "DMA_CAP", "0x380000", "0xAC", "0x3800AC", "RD_DONE_INT_CLEAR", 0, 8, "other", "0x0"],
    ["pcie_dma_write_test", "DMA_READ_INT_CLEAR_OFF", "DMA_CAP", "0x380000", "0xAC", "0x3800AC", "RD_ABORT_INT_CLEAR", 16, 8, "other", "0x0"],
    ["pcie_dma_write_test", "NA", "NA", "NA", "NA", "0xE6004100", "NA", "NA", "NA", "NA", "NA"],
    # TC3: COHERENCY_CONTROL_3_OFF fields
    ["pcie_mem_wr_rd_test", "COHERENCY_CONTROL_3_OFF", "PORT_LOGIC", "0x700", "0x1E8", "0x8E8", "CFG_MSTR_ARCACHE_MODE", 3, 4, "rw", "0x0"],
    ["pcie_mem_wr_rd_test", "COHERENCY_CONTROL_3_OFF", "PORT_LOGIC", "0x700", "0x1E8", "0x8E8", "CFG_MSTR_AWCACHE_MODE", 11, 4, "rw", "0x0"],
    ["pcie_mem_wr_rd_test", "COHERENCY_CONTROL_3_OFF", "PORT_LOGIC", "0x700", "0x1E8", "0x8E8", "CFG_MSTR_ARCACHE_VALUE", 19, 4, "rw", "0x0"],
    ["pcie_mem_wr_rd_test", "COHERENCY_CONTROL_3_OFF", "PORT_LOGIC", "0x700", "0x1E8", "0x8E8", "CFG_MSTR_AWCACHE_VALUE", 27, 4, "rw", "0x0"],
    ["pcie_mem_wr_rd_test", "NA", "NA", "NA", "NA", "0xE6004100", "NA", "NA", "NA", "NA", "NA"],
    # TC4: Register RW test registers
    ["pcie_reg_wr_rd_test", "MSI_CAP_OFF_08H_REG", "MSI_CAP", "0x50", "0x8", "0x58", "PCI_MSI_CAP_OFF_08H", 0, 16, "rw", "0x0"],
    ["pcie_reg_wr_rd_test", "MSI_CAP_OFF_08H_REG", "MSI_CAP", "0x50", "0x8", "0x58", "PCI_MSI_CAP_OFF_0AH", 16, 16, "rw", "0x0"],
    ["pcie_reg_wr_rd_test", "MSI_CAP_OFF_10H_REG", "MSI_CAP", "0x50", "0x10", "0x60", "PCI_MSI_CAP_OFF_10H", 0, 32, "rw", "0x0"],
    ["pcie_reg_wr_rd_test", "FILTER_MASK_2_OFF", "PORT_LOGIC", "0x700", "0x20", "0x720", "MASK_RADM_2", 0, 32, "rw", "0x0"],
    ["pcie_reg_wr_rd_test", "AXI_MSTR_MSG_ADDR_HIGH_OFF", "PORT_LOGIC", "0x700", "0x1F4", "0x8F4", "CFG_AXIMSTR_MSG_ADDR_HIGH", 0, 32, "rw", "0x0"],
    ["pcie_reg_wr_rd_test", "UTILITY_OFF", "PORT_LOGIC", "0x700", "0x580", "0xC80", "UTILITY", 0, 32, "rw", "0x0"],
    ["pcie_reg_wr_rd_test", "NA", "NA", "NA", "NA", "NA", "mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2", "NA", "NA", "NA", "NA"],
    ["pcie_reg_wr_rd_test", "NA", "NA", "NA", "NA", "NA", "mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3", "NA", "NA", "NA", "NA"],
    ["pcie_reg_wr_rd_test", "NA", "NA", "NA", "NA", "NA", "mizar_PCIE0_SII_PHY_CONTROL_23", "NA", "NA", "NA", "NA"],
    ["pcie_reg_wr_rd_test", "NA", "NA", "NA", "NA", "0xE68860B8", "NA", "NA", "NA", "NA", "NA"],
    ["pcie_reg_wr_rd_test", "NA", "NA", "NA", "NA", "0xE68862B8", "NA", "NA", "NA", "NA", "NA"],
    ["pcie_reg_wr_rd_test", "NA", "NA", "NA", "NA", "0xE68864B8", "NA", "NA", "NA", "NA", "NA"],
]

# Create workbook
wb = Workbook()

# --- TestPlan Sheet ---
ws1 = wb.active
ws1.title = 'TestPlan'

tp_headers = ['Index', 'SS / Module', 'Test Case Name', 'Feature', 'Test Description',
              'Test Type', 'Test Steps / Procedure', 'Impacted Registers',
              'Validation / Acceptance Criteria', 'Remarks',
              'Meta Test Description', 'Meta Test Steps / Procedure', 'Meta Impacted Registers']

header_font = Font(bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_align = Alignment(wrap_text=True, vertical='top')
thin_border = Border(
    left=Side(style='thin'), right=Side(style='thin'),
    top=Side(style='thin'), bottom=Side(style='thin')
)

# Write headers
for col, header in enumerate(tp_headers, 1):
    cell = ws1.cell(row=1, column=col, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align
    cell.border = thin_border

# Write data
for row_idx, tc in enumerate(testcases, 2):
    for col_idx, key in enumerate(tp_headers, 1):
        val = tc.get(key, '')
        cell = ws1.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_align
        cell.border = thin_border

# Auto-size columns
col_widths = [8, 12, 30, 35, 60, 12, 60, 40, 60, 50, 60, 60, 50]
for i, w in enumerate(col_widths, 1):
    ws1.column_dimensions[chr(64+i) if i <= 26 else 'A' + chr(64+i-26)].width = w

ws1.freeze_panes = 'A2'

# --- Register Details Sheet ---
ws2 = wb.create_sheet('Register Details')

rd_headers = ['Test Case Name', 'Register Name', 'Block', 'Block Base',
              'Register Offset (within block)', 'Absolute Offset',
              'Field Name', 'Bit Position', 'Field Width', 'Access Type', 'Reset Value']

for col, header in enumerate(rd_headers, 1):
    cell = ws2.cell(row=1, column=col, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_align
    cell.border = thin_border

for row_idx, rd in enumerate(register_details, 2):
    for col_idx, val in enumerate(rd, 1):
        cell = ws2.cell(row=row_idx, column=col_idx, value=val)
        cell.alignment = wrap_align
        cell.border = thin_border

rd_widths = [28, 30, 14, 12, 22, 14, 30, 12, 12, 12, 12]
for i, w in enumerate(rd_widths, 1):
    ws2.column_dimensions[chr(64+i) if i <= 26 else 'A' + chr(64+i-26)].width = w

ws2.freeze_panes = 'A2'

# Save
wb.save(output_path)
print(f'SUCCESS: Generated {output_path}')
print(f'FILENAME: {filename}')
print(f'ROWS_TESTPLAN: {len(testcases)}')
print(f'ROWS_REGISTER_DETAILS: {len(register_details)}')
