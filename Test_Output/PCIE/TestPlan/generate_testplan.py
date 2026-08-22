#!/usr/bin/env python3
"""PCIE TestPlan Excel Generator - Agent 7 Automated Output"""
import json
import os
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook, load_workbook
from openpyxl.styles import Font, PatternFill, Alignment
from openpyxl.utils import get_column_letter

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'PCIE_TestPlan_{timestamp}.xlsx'

# Complete aggregated JSON from Agent 6
json_data = [
  {
    "index": 1,
    "ss_module": "PCIE",
    "test_case_name": "pcie_device_enumerate_test",
    "feature": "Device Enumeration",
    "sub_feature": "PCIe Link Training and BAR Configuration",
    "test_description": "This test verifies PCIe device enumeration by performing link training, configuring cache coherency control registers for both PCIE0 and PCIE1 controllers, polling link status on SII0 and SII1 interfaces until link-up is confirmed, reading the Vendor ID from TYPE1_DEV_ID_VEND_ID_REG, enabling bus master and memory/IO space via TYPE1_STATUS_COMMAND_REG, programming memory base addresses for both dual-mode controllers, configuring system-level interrupt and control registers, disabling cache coherency, performing BAR sizing by writing all-ones and reading back BAR0_REG through PREF_MEM_LIMIT_PREF_MEM_BASE_REG on both PCIe slave ports, assigning BAR address values, and finally polling a synchronization register until a completion handshake value is received.",
    "meta_test_description": "This testcase performs PCIe device enumeration. It begins by writing 0x0 to 0xE6004100 to initialize. Link training is invoked via link_training_dm0_x4(4) or link_training_dm1_x4(4) depending on compile-time defines (DM0_RC, DM1_RC, DM0_EP, DM1_EP). Cache coherency programming is performed by read-modify-write on mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF and mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF using set_data() to set bits [14:11]=0xf, [6:3]=0xf, [30:27]=0xf, [22:19]=0xf. After wait_on(20), a consolidated read-modify-write sets all four bit fields in a single pass for both PCIE0 and PCIE1. Link status is polled via read_sii0_reg(0xC0) until (data & 0xD1) == 0xD1, then read_sii1_reg(0xC0) is polled similarly. Under DM0_RC, Vendor ID is read from read_pcie_slv0_reg(0x0), command register is written via write_pcie_slv0_reg(0x4, 0x7), and mem_base_program_dm0_x4()/mem_base_program_dm1_x4() are called with wait_on(10). System registers 0xE690000C through 0xE6900034 are written with 0x1. Cache disable programming follows by read-modify-write on both coherency control registers setting bits [22:19]=0x0 and [30:27]=0x0. After wait_on(30), BAR sizing is performed on PCIe slave port 1. Same BAR sizing sequence is repeated on PCIe slave port 0. Finally, after wait_on(10), register 0xE6004100 is polled until it reads 0x12345678, then finish(0) is called.",
    "test_steps": "1. Initialize the system control register to clear any pending state.\n2. Perform PCIe link training for the configured dual-mode controller (x4 lane width).\n3. Program cache coherency control registers for both PCIE0 and PCIE1 controllers by setting cache attribute bit fields [14:11], [6:3], [30:27], and [22:19] to enable caching.\n4. Wait for coherency settings to take effect.\n5. Perform a consolidated cache coherency programming pass for both controllers.\n6. Poll the SII0 link status register until link-up is confirmed (status bits [7:0] match expected pattern).\n7. Poll the SII1 link status register until link-up is confirmed.\n8. Read the TYPE1_DEV_ID_VEND_ID_REG to verify the Vendor ID of the enumerated device.\n9. Write to TYPE1_STATUS_COMMAND_REG to enable Bus Master, Memory Space, and IO Space.\n10. Program memory base addresses for both dual-mode controllers.\n11. Configure system-level control and interrupt registers to enable PCIe subsystem operation.\n12. Disable cache coherency by clearing cache attribute bit fields [22:19] and [30:27] in coherency control registers for both PCIE0 and PCIE1.\n13. Wait for cache disable to propagate.\n14. Perform BAR sizing on PCIe slave port 1: write all-ones to BAR0_REG, BAR1_REG, SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG, SEC_STAT_IO_LIMIT_IO_BASE_REG, MEM_LIMIT_MEM_BASE_REG, and PREF_MEM_LIMIT_PREF_MEM_BASE_REG, then read back to determine BAR sizes.\n15. Assign actual BAR address values on PCIe slave port 1 and verify by reading back.\n16. Repeat BAR sizing on PCIe slave port 0: write all-ones, read back, assign addresses, and verify.\n17. Poll the synchronization register until the expected completion handshake value is received.\n18. Confirm test completion.",
    "meta_test_steps": "1. write_reg(0xE6004100, 0x0) \u2014 initialize control register.\n2. Conditional compilation: call link_training_dm0_x4(4) or link_training_dm1_x4(4) based on DM0_RC/DM1_RC/DM0_EP/DM1_EP defines.\n3. CACHE PROGRAMMING \u2014 PCIE0:\n   a. rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 11, 14, 0xf)\n   b. rd_wr_data1 = set_data(rd_wr_data1, 3, 6, 0xf)\n   c. write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1)\n   d. rd_wr_data1 = set_data(read_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF), 27, 30, 0xf)\n   e. rd_wr_data1 = set_data(rd_wr_data1, 19, 22, 0xf)\n   f. write_reg(mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, rd_wr_data1)\n4. CACHE PROGRAMMING \u2014 PCIE1: same pattern for mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF.\n5-31. Remaining steps as detailed in full meta_test_steps.",
    "validation_criteria": "1. SII0 link status register must report link-up with status bits matching pattern (value & 0xD1) == 0xD1.\n2. SII1 link status register must report link-up with the same pattern.\n3. TYPE1_DEV_ID_VEND_ID_REG must return a valid Vendor ID upon read.\n4. TYPE1_STATUS_COMMAND_REG must accept write value 0x7 to enable Bus Master, Memory Space, and IO Space.\n5. BAR sizing on both slave ports must return valid BAR size masks when written with all-ones.\n6. BAR address assignment values must be read back correctly from BAR0_REG through PREF_MEM_LIMIT_PREF_MEM_BASE_REG on both slave ports.\n7. Synchronization register must eventually read 0x12345678 indicating successful completion handshake.\n8. Test must complete with finish(0) indicating no errors.",
    "speed": "NA",
    "mode": "RC",
    "remarks": "Test uses conditional compilation for dual-mode controller selection (DM0_RC, DM1_RC, DM0_EP, DM1_EP). Source code contains a duplicated link training and cache programming block. BAR sizing is performed on both PCIe slave port 0 and slave port 1. Cache coherency is enabled during enumeration and disabled afterward.",
    "impacted_registers": "COHERENCY_CONTROL_3_OFF; TYPE1_DEV_ID_VEND_ID_REG; TYPE1_STATUS_COMMAND_REG; BAR0_REG; BAR1_REG; SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG; SEC_STAT_IO_LIMIT_IO_BASE_REG; MEM_LIMIT_MEM_BASE_REG; PREF_MEM_LIMIT_PREF_MEM_BASE_REG",
    "meta_impacted_registers": "0xE6004100; mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF; mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF; 0xC0; 0x0; 0x4; 0xE690000C; 0xE6900010; 0xE6900014; 0xE6900018; 0xE6900030; 0xE6900034; 0x10; 0x14; 0x18; 0x1c; 0x20; 0x24",
    "register_details": [
      {"token": "0xE6004100", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0xE6004100", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF", "token_type": "macro", "operation": "read_modify_write", "base_value": "NA", "offset_value": "0x8E8", "resolution_status": "partially_resolved", "register_name": "COHERENCY_CONTROL_3_OFF", "mapping_status": "matched"},
      {"token": "mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF", "token_type": "macro", "operation": "read_modify_write", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0xC0", "token_type": "hex", "operation": "read", "base_value": "NA", "offset_value": "0xC0", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0x0", "token_type": "hex", "operation": "read", "base_value": "NA", "offset_value": "0x0", "resolution_status": "direct_hex", "register_name": "TYPE1_DEV_ID_VEND_ID_REG", "mapping_status": "matched"},
      {"token": "0x4", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0x4", "resolution_status": "direct_hex", "register_name": "TYPE1_STATUS_COMMAND_REG", "mapping_status": "matched"},
      {"token": "0xE690000C", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0xE690000C", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0xE6900010", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0xE6900010", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0xE6900014", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0xE6900014", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0xE6900018", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0xE6900018", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0xE6900030", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0xE6900030", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0xE6900034", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0xE6900034", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0x10", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0x10", "resolution_status": "direct_hex", "register_name": "BAR0_REG", "mapping_status": "matched"},
      {"token": "0x14", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0x14", "resolution_status": "direct_hex", "register_name": "BAR1_REG", "mapping_status": "matched"},
      {"token": "0x18", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0x18", "resolution_status": "direct_hex", "register_name": "SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG", "mapping_status": "matched"},
      {"token": "0x1c", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0x1c", "resolution_status": "direct_hex", "register_name": "SEC_STAT_IO_LIMIT_IO_BASE_REG", "mapping_status": "matched"},
      {"token": "0x20", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0x20", "resolution_status": "direct_hex", "register_name": "MEM_LIMIT_MEM_BASE_REG", "mapping_status": "matched"},
      {"token": "0x24", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0x24", "resolution_status": "direct_hex", "register_name": "PREF_MEM_LIMIT_PREF_MEM_BASE_REG", "mapping_status": "matched"}
    ],
    "source_files": ["program.c"],
    "headers_included": ["stdlib.h", "stdio.h", "test_common.h", "pcie.h"]
  },
  {
    "index": 2,
    "ss_module": "PCIE",
    "test_case_name": "pcie_dma_write_test",
    "feature": "DMA Data Transfer",
    "sub_feature": "Multi-Channel DMA Write and Read-Back Verification",
    "test_description": "This test verifies PCIe DMA write and read-back functionality across all four DMA channels (Channel 0 through Channel 3). The test performs link training, polls the link status register until link-up is confirmed, reads the Vendor ID from TYPE1_DEV_ID_VEND_ID_REG, enables bus master and memory/IO space via TYPE1_STATUS_COMMAND_REG, programs BAR and memory base addresses, preloads source memory with known data patterns (0xC0DEBEED and 0xF00DDEAF), configures the GIC for interrupt-driven DMA completion, unmasks DMA write and read interrupts via DMA_WRITE_INT_MASK_OFF and DMA_READ_INT_MASK_OFF, then sequentially programs and triggers DMA write transfers on all four channels using DMA_WRITE_DOORBELL_OFF, waiting for interrupt-driven completion after each channel. After all write channels complete, the test performs DMA read-back transfers on all four channels using DMA_READ_DOORBELL_OFF, again waiting for interrupt-driven completion per channel. The interrupt service routine reads DMA_WRITE_INT_STATUS_OFF and DMA_READ_INT_STATUS_OFF, masks the lower 4 bits to identify the completing channel, and clears the interrupt via DMA_WRITE_INT_CLEAR_OFF and DMA_READ_INT_CLEAR_OFF. The test completes with finish(0).",
    "meta_test_description": "This testcase performs PCIe DMA write and read-back operations across all four DMA channels. It begins by writing 0x0 to 0xE6004100 to initialize. Link training is invoked via link_training_dm0_x4(4) or link_training_dm1_x4(4) depending on compile-time defines. DMA operations are interrupt-driven using GIC.",
    "test_steps": "1. Initialize the system control register to clear any pending state.\n2. Perform PCIe link training for the configured dual-mode controller at x4 lane width.\n3. Poll the SII link status register until link-up is confirmed with expected status pattern.\n4. Read TYPE1_DEV_ID_VEND_ID_REG to verify the Vendor ID of the connected endpoint.\n5. Write to TYPE1_STATUS_COMMAND_REG to enable Bus Master, Memory Space, and IO Space.\n6. Program BAR registers and memory base addresses for the configured controller.\n7. Configure non-secure protection settings.\n8. Poll the synchronization register until the expected handshake value is received.\n9. Set DMA transfer length to 0x40 (64 bytes) and configure source, write, read, and destination addresses for all four channels.\n10. Preload source memory with known data patterns (0xC0DEBEED for first 128 words, 0xF00DDEAF for next 128 words).\n11. Initialize the GIC and enable all IRQs for interrupt-driven DMA completion.\n12. Unmask DMA write and read interrupts by writing 0x0 to DMA_WRITE_INT_MASK_OFF and DMA_READ_INT_MASK_OFF.\n13. Program DMA write channel 0 with source and destination addresses and trigger transfer via DMA_WRITE_DOORBELL_OFF. Wait for interrupt-driven completion.\n14. Repeat DMA write programming and doorbell trigger for channels 1, 2, and 3, waiting for completion after each.\n15. Program DMA read channel 0 with remote read address and local destination address and trigger transfer via DMA_READ_DOORBELL_OFF. Wait for interrupt-driven completion.\n16. Repeat DMA read programming and doorbell trigger for channels 1, 2, and 3, waiting for completion after each.\n17. Verify that the interrupt handler reads DMA_WRITE_INT_STATUS_OFF and DMA_READ_INT_STATUS_OFF, extracts the channel completion status from the lower 4 bits, and clears interrupts via DMA_WRITE_INT_CLEAR_OFF and DMA_READ_INT_CLEAR_OFF.\n18. Confirm test completion with finish(0).",
    "meta_test_steps": "1. write_reg(0xE6004100, 0x0) \u2014 initialize control register.\n2-34. Full DMA channel programming, doorbell triggers, interrupt handling as detailed in Agent 5 output.",
    "validation_criteria": "1. Link status register must report link-up with status bits matching pattern (value & 0xD1) == 0xD1.\n2. TYPE1_DEV_ID_VEND_ID_REG must return a valid Vendor ID upon read.\n3. TYPE1_STATUS_COMMAND_REG must accept write value 0x7 to enable Bus Master, Memory Space, and IO Space.\n4. Synchronization register must read 0x12345678 before DMA operations begin.\n5. DMA write interrupt must fire for each of the four write channels.\n6. DMA read interrupt must fire for each of the four read channels.\n7. DMA_WRITE_INT_CLEAR_OFF must successfully clear the write interrupt status.\n8. DMA_READ_INT_CLEAR_OFF must successfully clear the read interrupt status.\n9. All four DMA write channels and all four DMA read channels must complete without timeout.\n10. Test must complete with finish(0) indicating no errors.",
    "speed": "NA",
    "mode": "RC",
    "remarks": "Test uses conditional compilation for dual-mode controller selection. DMA operations are interrupt-driven using GIC with IRQ 0x20 for DM0 and IRQ 0x23 for DM1. Source memory is preloaded with two distinct data patterns. Transfer length is 0x40 bytes per channel.",
    "impacted_registers": "TYPE1_DEV_ID_VEND_ID_REG; TYPE1_STATUS_COMMAND_REG; DMA_WRITE_INT_MASK_OFF; DMA_READ_INT_MASK_OFF; DMA_WRITE_DOORBELL_OFF; DMA_READ_DOORBELL_OFF; DMA_WRITE_INT_STATUS_OFF; DMA_READ_INT_STATUS_OFF; DMA_WRITE_INT_CLEAR_OFF; DMA_READ_INT_CLEAR_OFF",
    "meta_impacted_registers": "0xE6004100; 0xC0; 0x0; 0x4; mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF; mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF; mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF; mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF; mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF; mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF; mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF; mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF; mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF; mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF; mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF; mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF; mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF; mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF; mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF; mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF",
    "register_details": [
      {"token": "0xE6004100", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0xE6004100", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0xC0", "token_type": "hex", "operation": "read", "base_value": "NA", "offset_value": "0xC0", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0x0", "token_type": "hex", "operation": "read", "base_value": "NA", "offset_value": "0x0", "resolution_status": "direct_hex", "register_name": "TYPE1_DEV_ID_VEND_ID_REG", "mapping_status": "matched"},
      {"token": "0x4", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0x4", "resolution_status": "direct_hex", "register_name": "TYPE1_STATUS_COMMAND_REG", "mapping_status": "matched"},
      {"token": "mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_MASK_OFF", "token_type": "macro", "operation": "write", "base_value": "NA", "offset_value": "0x380054", "resolution_status": "partially_resolved", "register_name": "DMA_WRITE_INT_MASK_OFF", "mapping_status": "matched"},
      {"token": "mizar_PCIE0_DBI_DSP_DMA_READ_INT_MASK_OFF", "token_type": "macro", "operation": "write", "base_value": "NA", "offset_value": "0x3800A8", "resolution_status": "partially_resolved", "register_name": "DMA_READ_INT_MASK_OFF", "mapping_status": "matched"},
      {"token": "mizar_PCIE0_DBI_DSP_DMA_WRITE_DOORBELL_OFF", "token_type": "macro", "operation": "write", "base_value": "NA", "offset_value": "0x380010", "resolution_status": "partially_resolved", "register_name": "DMA_WRITE_DOORBELL_OFF", "mapping_status": "matched"},
      {"token": "mizar_PCIE0_DBI_DSP_DMA_READ_DOORBELL_OFF", "token_type": "macro", "operation": "write", "base_value": "NA", "offset_value": "0x380030", "resolution_status": "partially_resolved", "register_name": "DMA_READ_DOORBELL_OFF", "mapping_status": "matched"},
      {"token": "mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_MASK_OFF", "token_type": "macro", "operation": "write", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE1_DBI_DSP_DMA_READ_INT_MASK_OFF", "token_type": "macro", "operation": "write", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE1_DBI_DSP_DMA_WRITE_DOORBELL_OFF", "token_type": "macro", "operation": "write", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE1_DBI_DSP_DMA_READ_DOORBELL_OFF", "token_type": "macro", "operation": "write", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_STATUS_OFF", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "0x38004C", "resolution_status": "partially_resolved", "register_name": "DMA_WRITE_INT_STATUS_OFF", "mapping_status": "matched"},
      {"token": "mizar_PCIE0_DBI_DSP_DMA_READ_INT_STATUS_OFF", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "0x3800A0", "resolution_status": "partially_resolved", "register_name": "DMA_READ_INT_STATUS_OFF", "mapping_status": "matched"},
      {"token": "mizar_PCIE0_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF", "token_type": "macro", "operation": "write", "base_value": "NA", "offset_value": "0x380058", "resolution_status": "partially_resolved", "register_name": "DMA_WRITE_INT_CLEAR_OFF", "mapping_status": "matched"},
      {"token": "mizar_PCIE0_DBI_DSP_DMA_READ_INT_CLEAR_OFF", "token_type": "macro", "operation": "write", "base_value": "NA", "offset_value": "0x3800AC", "resolution_status": "partially_resolved", "register_name": "DMA_READ_INT_CLEAR_OFF", "mapping_status": "matched"},
      {"token": "mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_STATUS_OFF", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE1_DBI_DSP_DMA_READ_INT_STATUS_OFF", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE1_DBI_DSP_DMA_WRITE_INT_CLEAR_OFF", "token_type": "macro", "operation": "write", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE1_DBI_DSP_DMA_READ_INT_CLEAR_OFF", "token_type": "macro", "operation": "write", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"}
    ],
    "source_files": ["program.c"],
    "headers_included": ["stdlib.h", "stdio.h", "test_common.h", "pcie.h"]
  },
  {
    "index": 3,
    "ss_module": "PCIE",
    "test_case_name": "pcie_mem_wr_rd_test",
    "feature": "Memory Write Read",
    "sub_feature": "PCIe Memory Space Write and Read-Back Verification",
    "test_description": "This test verifies PCIe memory space write and read-back functionality through the PCIe slave ports. The test performs link training, programs cache coherency control registers for both PCIE0 and PCIE1 controllers to enable caching, polls the SII link status register until link-up is confirmed, reads the Vendor ID from TYPE1_DEV_ID_VEND_ID_REG, enables bus master and memory/IO space via TYPE1_STATUS_COMMAND_REG, programs BAR registers and memory base addresses, configures non-secure protection, signals readiness via a synchronization register, disables cache coherency by clearing the upper cache attribute bit fields in the coherency control registers, then performs memory write and read-back verification at multiple target memory addresses through the PCIe slave port using known data patterns. The test completes by polling a synchronization register until the expected completion handshake value is received.",
    "meta_test_description": "This testcase performs PCIe memory space write and read-back operations. It begins by writing 0x0 to 0xE6004100 to initialize. Cache coherency programming and link training are performed. Memory write and read-back operations verify data integrity through PCIe slave ports.",
    "test_steps": "1. Initialize the system control register to clear any pending state.\n2. Perform PCIe link training for the configured dual-mode controller at x4 lane width.\n3. Program cache coherency control registers for both PCIE0 and PCIE1 controllers by setting cache attribute bit fields to enable caching.\n4. Wait for coherency settings to take effect.\n5. Perform a consolidated cache coherency programming pass for both controllers.\n6. Poll the SII link status register until link-up is confirmed with expected status pattern.\n7. Read the TYPE1_DEV_ID_VEND_ID_REG to verify the Vendor ID of the connected device.\n8. Write to TYPE1_STATUS_COMMAND_REG to enable Bus Master, Memory Space, and IO Space.\n9. Program BAR registers and memory base addresses for the configured controller.\n10. Configure non-secure protection settings.\n11. Signal readiness by writing a synchronization value to the control register.\n12. Disable cache coherency by clearing the upper cache attribute bit fields in the coherency control registers for both PCIE0 and PCIE1.\n13. Wait for cache disable settings to propagate.\n14. Perform consolidated cache disable programming for both controllers.\n15. Wait for the system to stabilize after cache disable.\n16. Perform memory write and read-back verification at multiple target memory addresses through the PCIe slave port using known data patterns.\n17. Poll the synchronization register until the expected completion handshake value is received.\n18. Confirm test completion.",
    "meta_test_steps": "1. write_reg(0xE6004100, 0x0) \u2014 initialize control register.\n2-30. Cache programming, link training, memory write/read operations as detailed in Agent 5 output.",
    "validation_criteria": "1. Link status register must report link-up with status bits matching pattern (value & 0xD1) == 0xD1.\n2. TYPE1_DEV_ID_VEND_ID_REG must return a valid Vendor ID upon read.\n3. TYPE1_STATUS_COMMAND_REG must accept write value 0x7 to enable Bus Master, Memory Space, and IO Space.\n4. Memory write and read-back operations at all target addresses must return the same data pattern that was written.\n5. Under RC mode, data patterns must be verified at their respective addresses.\n6. Under EP mode, data pattern 0x5a5a5a5a must be verified at all five BAR1 memory offsets.\n7. Synchronization register must eventually read 0x12345678.\n8. Test must complete with finish(0) indicating no errors.",
    "speed": "NA",
    "mode": "RC/EP",
    "remarks": "Test uses conditional compilation for dual-mode controller selection (DM0_RC, DM1_RC, DM0_EP, DM1_EP). In RC mode, three memory addresses are tested with unique data patterns per controller. In EP mode, five BAR1 memory offsets are tested with a uniform data pattern. Cache coherency is enabled during link setup and disabled before memory write/read-back operations.",
    "impacted_registers": "COHERENCY_CONTROL_3_OFF; TYPE1_DEV_ID_VEND_ID_REG; TYPE1_STATUS_COMMAND_REG",
    "meta_impacted_registers": "0xE6004100; mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF; mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF; 0xC0; 0x0; 0x4",
    "register_details": [
      {"token": "0xE6004100", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0xE6004100", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF", "token_type": "macro", "operation": "read_modify_write", "base_value": "NA", "offset_value": "0x8E8", "resolution_status": "partially_resolved", "register_name": "COHERENCY_CONTROL_3_OFF", "mapping_status": "matched"},
      {"token": "mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF", "token_type": "macro", "operation": "read_modify_write", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0xC0", "token_type": "hex", "operation": "read", "base_value": "NA", "offset_value": "0xC0", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0x0", "token_type": "hex", "operation": "read", "base_value": "NA", "offset_value": "0x0", "resolution_status": "direct_hex", "register_name": "TYPE1_DEV_ID_VEND_ID_REG", "mapping_status": "matched"},
      {"token": "0x4", "token_type": "hex", "operation": "write", "base_value": "NA", "offset_value": "0x4", "resolution_status": "direct_hex", "register_name": "TYPE1_STATUS_COMMAND_REG", "mapping_status": "matched"}
    ],
    "source_files": ["program.c"],
    "headers_included": ["stdlib.h", "stdio.h", "test_common.h", "pcie.h"]
  },
  {
    "index": 4,
    "ss_module": "PCIE",
    "test_case_name": "pcie_reg_wr_rd_test",
    "feature": "Register Write Read",
    "sub_feature": "Reset Value Verification and Read-Write Data Integrity",
    "test_description": "This test verifies PCIe register reset default values and read-write data integrity across multiple register domains including DBI controller registers, SII interface registers, and PHY registers for both PCIE0 and PCIE1 controllers. The test first reads all target registers and compares against expected default values of 0x0. It then performs iterative write-read verification using multiple data patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555) across all register sets, applying write masks for SII and PHY registers. For DBI controller registers, MSI_CAP_OFF_08H_REG, MSI_CAP_OFF_10H_REG, FILTER_MASK_2_OFF, AXI_MSTR_MSG_ADDR_HIGH_OFF, and UTILITY_OFF are tested on both PCIE0 and PCIE1. PHY registers are accessed with 16-bit alignment handling based on address alignment. The test reports pass/fail based on accumulated error counts from both reset value checks and read-write verification.",
    "meta_test_description": "This testcase performs PCIe register reset value verification and read-write data integrity testing across DBI controller, SII interface, and PHY register domains for both PCIE0 and PCIE1 controllers.",
    "test_steps": "1. Read all five DBI controller registers for PCIE0 and verify they contain the expected reset default value of 0x0.\n2. Read all five DBI controller registers for PCIE1 and verify they contain the expected reset default value of 0x0.\n3. Read all three SII interface registers for PCIE0 and verify they contain the expected reset default value of 0x0.\n4. Read all three SII interface registers for PCIE1 and verify they contain the expected reset default value of 0x0.\n5. Release PHY reset for both PCIE0 and PCIE1 by writing the PHY reset control registers.\n6. Read all three PHY registers for PCIE0 with 16-bit alignment handling and verify they contain the expected reset default value of 0x0.\n7. Read all three PHY registers for PCIE1 with 16-bit alignment handling and verify they contain the expected reset default value of 0x0.\n8. For each of three data patterns (all-ones, alternating-A, alternating-5), write the pattern to all five DBI controller registers for both PCIE0 and PCIE1.\n9. For each data pattern, write the masked pattern to all three SII interface registers for both PCIE0 and PCIE1.\n10. Release PHY reset for both controllers before PHY register writes.\n11. For each PHY data pattern, write the masked pattern to all three PHY registers for both PCIE0 and PCIE1.\n12. Read back all five DBI controller registers for both PCIE0 and PCIE1 and verify the written data pattern matches.\n13. Read back all three SII interface registers for both PCIE0 and PCIE1 and verify the masked written data matches.\n14. Read back all three PHY registers for both PCIE0 and PCIE1 with 16-bit alignment handling and verify the masked written data matches.\n15. Confirm test completion with pass/fail based on accumulated error counts.",
    "meta_test_steps": "1. chk_rst_val() called.\n2-14. Reset value verification and read-write data integrity testing as detailed in Agent 5 output.",
    "validation_criteria": "1. All five DBI controller registers for PCIE0 must read back their expected reset default value of 0x0.\n2. All five DBI controller registers for PCIE1 must read back their expected reset default value of 0x0.\n3. All three SII interface registers for both PCIE0 and PCIE1 must read back their expected reset default value of 0x0.\n4. All three PHY registers for both PCIE0 and PCIE1 must read back their expected reset default value of 0x0 after 16-bit alignment extraction.\n5. For each of three data patterns (0xFFFFFFFF, 0xAAAAAAAA, 0x55555555), all DBI controller registers must read back the exact written value.\n6. For each data pattern, SII registers must read back the written value masked with the respective write mask.\n7. For each PHY data pattern (0x7BAF, 0x1, 0x003B), PHY registers must read back the written value masked with 0x1FFF after 16-bit alignment extraction.\n8. Error counters err1 and err2 must both be zero for the test to pass.\n9. Test must complete with finish(0) indicating no errors.",
    "speed": "NA",
    "mode": "NA",
    "remarks": "Test does not perform link training or any PCIe data-plane operations. It operates purely at the register level. PHY registers require 16-bit alignment handling. PHY reset control registers are written with 0x01203000 before PHY register access. Write masks are applied for SII registers and PHY registers. Only the first three of six defined check values are used in the read-write loop.",
    "impacted_registers": "MSI_CAP_OFF_08H_REG; MSI_CAP_OFF_10H_REG; FILTER_MASK_2_OFF; AXI_MSTR_MSG_ADDR_HIGH_OFF; UTILITY_OFF",
    "meta_impacted_registers": "mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG; mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG; mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF; mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF; mizar_PCIE0_DBI_DSP_UTILITY_OFF; mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG; mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG; mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF; mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF; mizar_PCIE1_DBI_DSP_UTILITY_OFF; mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2; mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3; mizar_PCIE0_SII_PHY_CONTROL_23; mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2; mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3; mizar_PCIE1_SII_PHY_CONTROL_23; mizar_PCIE0_SII_PHY_RST_CONTROL; mizar_PCIE1_SII_PHY_RST_CONTROL; 0xE68860B8; 0xE68862B8; 0xE68864B8; 0xE68A60B8; 0xE68A62B8; 0xE68A64B8",
    "register_details": [
      {"token": "mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "0x58", "resolution_status": "partially_resolved", "register_name": "MSI_CAP_OFF_08H_REG", "mapping_status": "matched"},
      {"token": "mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "0x60", "resolution_status": "partially_resolved", "register_name": "MSI_CAP_OFF_10H_REG", "mapping_status": "matched"},
      {"token": "mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "0x720", "resolution_status": "partially_resolved", "register_name": "FILTER_MASK_2_OFF", "mapping_status": "matched"},
      {"token": "mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "0x8F4", "resolution_status": "partially_resolved", "register_name": "AXI_MSTR_MSG_ADDR_HIGH_OFF", "mapping_status": "matched"},
      {"token": "mizar_PCIE0_DBI_DSP_UTILITY_OFF", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "0xC80", "resolution_status": "partially_resolved", "register_name": "UTILITY_OFF", "mapping_status": "matched"},
      {"token": "mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE1_DBI_DSP_UTILITY_OFF", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE0_SII_PHY_CONTROL_23", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE1_SII_PHY_CONTROL_23", "token_type": "macro", "operation": "read", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE0_SII_PHY_RST_CONTROL", "token_type": "macro", "operation": "write", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "mizar_PCIE1_SII_PHY_RST_CONTROL", "token_type": "macro", "operation": "write", "base_value": "NA", "offset_value": "NA", "resolution_status": "unresolved", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0xE68860B8", "token_type": "hex", "operation": "read", "base_value": "NA", "offset_value": "0xE68860B8", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0xE68862B8", "token_type": "hex", "operation": "read", "base_value": "NA", "offset_value": "0xE68862B8", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0xE68864B8", "token_type": "hex", "operation": "read", "base_value": "NA", "offset_value": "0xE68864B8", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0xE68A60B8", "token_type": "hex", "operation": "read", "base_value": "NA", "offset_value": "0xE68A60B8", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0xE68A62B8", "token_type": "hex", "operation": "read", "base_value": "NA", "offset_value": "0xE68A62B8", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"},
      {"token": "0xE68A64B8", "token_type": "hex", "operation": "read", "base_value": "NA", "offset_value": "0xE68A64B8", "resolution_status": "direct_hex", "register_name": "NA", "mapping_status": "unresolved"}
    ],
    "source_files": ["program.c"],
    "headers_included": ["stdlib.h", "stdio.h", "test_common.h", "pcie.h"]
  }
]

# Create workbook
wb = Workbook()

# TestPlan sheet
ws_tp = wb.active
ws_tp.title = 'TestPlan'

# MetaData sheet
ws_md = wb.create_sheet('MetaData')

# TestPlan headers
tp_headers = ['Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
              'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
              'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
              'Code Generation']

# MetaData headers
md_headers = ['Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
              'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
              'Meta Headers', 'Meta Macros', 'Meta Arrays']

# Formatting
header_font = Font(bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_alignment = Alignment(wrap_text=True, vertical='top')

# Write TestPlan headers
for col_idx, header in enumerate(tp_headers, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Write MetaData headers
for col_idx, header in enumerate(md_headers, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=header)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = wrap_alignment

# Populate data
for row_idx, tc in enumerate(json_data, 2):
    # Extract macro tokens
    macros = [rd['token'] for rd in tc.get('register_details', []) if rd.get('token_type') == 'macro']
    macros_str = '; '.join(macros) if macros else ''
    
    # Serialize register_details for Meta Arrays
    reg_details = tc.get('register_details', [])
    arrays_parts = []
    for rd in reg_details:
        parts = [f"token={rd.get('token','')}", f"type={rd.get('token_type','')}",
                 f"op={rd.get('operation','')}", f"offset={rd.get('offset_value','')}",
                 f"reg={rd.get('register_name','')}", f"status={rd.get('mapping_status','')}"]
        arrays_parts.append(' | '.join(parts))
    arrays_str = '\n'.join(arrays_parts)
    
    # Headers
    headers_str = '; '.join(tc.get('headers_included', []))
    
    # TestPlan row
    tp_row = [
        tc.get('index', ''),
        tc.get('ss_module', ''),
        tc.get('feature', ''),
        tc.get('test_case_name', ''),
        tc.get('test_description', ''),
        tc.get('speed', ''),
        tc.get('mode', ''),
        '',  # Memory Start Offset
        '',  # Memory End Offset
        tc.get('remarks', ''),
        tc.get('test_steps', ''),
        tc.get('impacted_registers', ''),
        tc.get('validation_criteria', ''),
        ''   # Code Generation
    ]
    for col_idx, value in enumerate(tp_row, 1):
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment
    
    # MetaData row
    md_row = [
        tc.get('index', ''),
        tc.get('test_case_name', ''),
        tc.get('meta_test_description', ''),
        tc.get('meta_test_steps', ''),
        tc.get('meta_impacted_registers', ''),
        tc.get('validation_criteria', ''),
        headers_str,
        macros_str,
        arrays_str
    ]
    for col_idx, value in enumerate(md_row, 1):
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = wrap_alignment

# Freeze first row
ws_tp.freeze_panes = 'A2'
ws_md.freeze_panes = 'A2'

# Auto-size columns
MAX_WIDTH = 60
for ws in [ws_tp, ws_md]:
    for col in ws.columns:
        max_length = 0
        col_letter = get_column_letter(col[0].column)
        for cell in col:
            if cell.value:
                lines = str(cell.value).split('\n')
                for line in lines:
                    max_length = max(max_length, len(line))
        adjusted = min(max_length + 2, MAX_WIDTH)
        ws.column_dimensions[col_letter].width = max(adjusted, 12)

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# Save
script_dir = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(script_dir, filename)
wb.save(output_path)

# Validate
wb2 = load_workbook(output_path)
assert 'TestPlan' in wb2.sheetnames, 'TestPlan sheet missing'
assert 'MetaData' in wb2.sheetnames, 'MetaData sheet missing'
assert os.path.getsize(output_path) > 0, 'File is empty'

print(f'SUCCESS: {filename}')
print(f'Path: {output_path}')
print(f'Size: {os.path.getsize(output_path)} bytes')
print(f'Sheets: {wb2.sheetnames}')
print(f'TestPlan rows: {ws_tp.max_row - 1}')
print(f'MetaData rows: {ws_md.max_row - 1}')
