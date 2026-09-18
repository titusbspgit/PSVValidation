#!/usr/bin/env python3
"""Generate MIPI_DSI TestPlan Excel workbook using openpyxl."""
import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
except ImportError:
    import subprocess
    subprocess.check_call([sys.executable, '-m', 'pip', 'install', 'openpyxl'])
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp_str = now_ist.strftime('%Y%m%d_%H%M%S')

IP_NAME = 'MIPI_DSI'
filename = f'{IP_NAME}_TestPlan_{timestamp_str}.xlsx'
output_dir = os.environ.get('OUTPUT_DIR', 'Test_Output/MIPI/TestPlan')
os.makedirs(output_dir, exist_ok=True)
output_path = os.path.join(output_dir, filename)

# Input JSON data
json_data = [
    {
        "Index": "1",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_basic_test",
        "Feature": "NA",
        "Meta Headers": "<stdlib.h>; <stdio.h>; <test_common.h>; \"pcie.h\"",
        "Meta Macros": "DM0_RC; DM1_RC; DM0_EP; DM1_EP; DEBUG_DISPLAY",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "The testcase program.c performs the following: writes to a control register at a hardcoded address, conditionally invokes link training functions (link_training_dm0_x4 or link_training_dm1_x4) based on compile-time defines (DM0_RC, DM1_RC, DM0_EP, DM1_EP). It then performs cache programming by reading SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF and SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF registers, modifying bit fields using set_data, and writing them back. It calls non_secure_prot_nic(), polls read_sii0_reg and read_sii1_reg until specific status bits are set. Under DM0_RC, it reads vendor ID via read_pcie_slv0_reg, programs memory bases via mem_base_program_dm0_x4 and mem_base_program_dm1_x4, writes to multiple hardcoded addresses. It then disables cache programming by clearing bit fields in the coherency control registers. It writes BAR registers via write_pcie_slv1_reg and write_pcie_slv0_reg, reads them back, reprograms them with specific values, reads them back again. Finally, it polls a register at a hardcoded address until a specific value is read, then calls finish(0). Note: The source code in this folder does not contain MIPI DSI register operations. The Agent 2/3/4 outputs describe MIPI DSI registers resolved from header-level analysis.",
        "Test Description": "This test performs basic validation within the MIPI DSI test folder. The source code initializes link training, configures cache coherency control registers, polls status registers for link readiness, programs memory base addresses, configures BAR registers, and polls for a completion value before finishing. The test validates basic initialization and link-up sequences.",
        "Meta Test Steps / Procedure": "1. Write 0x0 to hardcoded control register. 2. Conditionally call link_training_dm0_x4(4) or link_training_dm1_x4(4) based on DM0_RC/DM1_RC/DM0_EP/DM1_EP defines. 3. Read SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, modify bit fields [11:14] and [3:6] with 0xF using set_data, write back. 4. Read SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF again, modify bit fields [27:30] and [19:22] with 0xF, write back. 5. Repeat steps 3-4 for SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF. 6. wait_on(20). 7. Repeat combined bit field modifications for both coherency control registers. 8. Call non_secure_prot_nic(). 9. Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1. 10. Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1. 11. Under DM0_RC: read vendor ID via read_pcie_slv0_reg(0x0), write command register, call mem_base_program_dm0_x4() and mem_base_program_dm1_x4(), wait_on(10). 12. Write to six hardcoded addresses. 13. Disable cache by clearing bit fields [27:30] and [19:22] to 0x0 in both coherency control registers. 14. wait_on(10), repeat disable cache with combined fields. 15. wait_on(30). 16. Write 0xFFFFFFFF to BAR registers via write_pcie_slv1_reg, read back, reprogram with specific values, read back. 17. Repeat BAR programming for pcie_slv0. 18. wait_on(10). 19. Poll hardcoded register until value equals 0x12345678. 20. Call finish(0).",
        "Test Steps / Procedure": "1. Initialize the system by writing to the control register. 2. Perform link training based on the configured device mode. 3. Configure cache coherency control registers for both PCIe ports by setting appropriate bit fields. 4. Wait for configuration to settle. 5. Call non-secure protection configuration. 6. Poll SII0 and SII1 status registers until link readiness bits are set. 7. Read the vendor ID register and program memory base addresses for both ports. 8. Write to subsystem control registers. 9. Disable cache by clearing coherency control bit fields for both ports. 10. Wait and confirm cache disable by re-clearing fields. 11. Program BAR registers for both PCIe slave ports with test patterns and verify readback. 12. Reprogram BAR registers with target address values and verify readback. 13. Poll completion status register until expected completion value is received. 14. Finish the test.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Impacted Registers": "MIPI_DSI_DMAC_INTEN; MIPI_DSI_SUBSYS_INTERRUPT_ENABLE; MIPI_DSI_HOST_PHY_IF_CFG; MIPI_DSI_HOST_PCKHDL_CFG; MIPI_DSI_HOST_CLKMGR_CFG; MIPI_DSI_SUBSYS_DPI_CONTROL; MIPI_DSI_DMAC_DBGINST0; MIPI_DSI_DMAC_DBGINST1; MIPI_DSI_DMAC_DBGCMD; MIPI_DSI_SUBSYS_INTERRUPT_MASK; MIPI_DSI_DMAC_INTMIS; MIPI_DSI_DMAC_INTCLR; MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Meta Validation / Acceptance Criteria": "The source code polls read_sii0_reg(0xC0) in a while loop until (data_rd & 0xD1) == 0xD1, and polls read_sii1_reg(0xC0) until the same condition is met. It then polls a hardcoded register until the read value equals 0x12345678. The test completes by calling finish(0), indicating a pass condition. BAR register readbacks are performed but no explicit comparison or assertion is present in the source code for those values.",
        "Validation / Acceptance Criteria": "The test passes when the SII0 and SII1 status registers report link readiness with the expected status bits set. The completion status register must return the expected completion value. The test calls the finish routine with a pass indicator upon successful completion of all polling conditions.",
        "Remarks": "The source code in the mipi_dsi_basic_test folder (program.c) contains PCIe link training and coherency control logic rather than MIPI DSI-specific operations. The MIPI DSI register information in the Meta Impacted Registers and Impacted Registers fields is derived from upstream Agent 2/3/4 outputs based on header-level analysis. There is a mismatch between the actual source code content and the MIPI DSI register context provided by upstream agents. Conditional compilation blocks (DM0_RC, DM1_RC, DM0_EP, DM1_EP) control link training and memory base programming paths. Multiple wait_on calls are used for timing synchronization."
    },
    {
        "Index": "2",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_dbi_random_payload_test",
        "Feature": "DBI Random Payload Transfer",
        "Meta Headers": "<stdlib.h>; <stdio.h>; <test_common.h>; \"pcie.h\"",
        "Meta Macros": "DM0_RC; DM1_RC; DM0_EP; DM1_EP; DEBUG_DISPLAY",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "The testcase program.c performs the following: writes 0x0 to a hardcoded control register at 0xE6004100. Conditionally invokes link training functions (link_training_dm0_x4 or link_training_dm1_x4) based on compile-time defines (DM0_RC, DM1_RC, DM0_EP, DM1_EP). Performs cache programming by reading SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF and SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF registers, modifying bit fields [11:14], [3:6], [27:30], [19:22] using set_data with value 0xF, and writing them back. Waits using wait_on(20), then repeats combined bit field modifications for both coherency control registers. Calls non_secure_prot_nic(). Polls read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1 and polls read_sii1_reg(0xC0) until the same condition is met. Under DM0_RC, reads vendor ID via read_pcie_slv0_reg(0x0), writes command register via write_pcie_slv0_reg(0x4, 0x7), calls mem_base_program_dm0_x4() and mem_base_program_dm1_x4(), waits. Writes to six hardcoded addresses at 0xE690000C through 0xE6900034 with value 0x1. Performs disable-cache programming by clearing bit fields [27:30] and [19:22] to 0x0 in both coherency control registers. Waits, then repeats disable-cache with combined fields. Waits 30 cycles. Programs BAR registers via write_pcie_slv1_reg and write_pcie_slv0_reg with 0xFFFFFFFF, reads them back, reprograms with specific address values (0x0, 0x4, 0x20000000, 0x40000000, 0x60000000, 0x80000000), reads them back again. Waits, then polls register at 0xE6004100 until value equals 0x12345678 with wait_on(5) between iterations. Calls finish(0). Note: The source code does not contain MIPI DSI register operations. The Agent 2/3/4 outputs describe MIPI DSI registers resolved from header-level analysis.",
        "Test Description": "This test validates DBI random payload transfer functionality within the MIPI DSI subsystem. The source code performs link training initialization, configures cache coherency control registers for both PCIe ports, polls status registers for link readiness, programs memory base addresses, writes to subsystem control registers, disables cache programming, programs BAR registers with test patterns and target addresses while verifying readback, and polls a completion status register until the expected completion value is received before finishing the test.",
        "Meta Test Steps / Procedure": "1. Write 0x0 to hardcoded control register at 0xE6004100. 2. Conditionally call link_training_dm0_x4(4) or link_training_dm1_x4(4) based on DM0_RC/DM1_RC/DM0_EP/DM1_EP defines. 3. Read SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, modify bit fields [11:14] and [3:6] with 0xF using set_data, write back. 4. Read SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF again, modify bit fields [27:30] and [19:22] with 0xF, write back. 5. Repeat steps 3-4 for SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF. 6. wait_on(20). 7. Repeat combined bit field modifications [11:14], [3:6], [27:30], [19:22] with 0xF for both coherency control registers. 8. Read read_sii0_reg(0xC0), call non_secure_prot_nic(). 9. Poll read_sii0_reg(0xC0) in while loop until (data_rd & 0xD1) == 0xD1. 10. Poll read_sii1_reg(0xC0) in while loop until (data_rd & 0xD1) == 0xD1. 11. Under DM0_RC: read vendor ID via read_pcie_slv0_reg(0x0), write command register via write_pcie_slv0_reg(0x4, 0x7), call mem_base_program_dm0_x4() and mem_base_program_dm1_x4(), wait_on(10). 12. Write 0x1 to six hardcoded addresses: 0xE690000C, 0xE6900010, 0xE6900014, 0xE6900018, 0xE6900030, 0xE6900034. 13. Disable cache: read-modify-write both coherency control registers clearing bit fields [27:30] and [19:22] to 0x0. 14. wait_on(10), repeat disable cache with combined fields clearing [27:30] and [19:22] to 0x0. 15. wait_on(30). 16. Write 0xFFFFFFFF to BAR registers at offsets 0x10-0x24 via write_pcie_slv1_reg, read back via read_pcie_slv1_reg. 17. Reprogram BAR registers with specific address values (0x0, 0x4, 0x20000000, 0x40000000, 0x60000000, 0x80000000), read back. 18. Repeat steps 16-17 for pcie_slv0. 19. wait_on(10). 20. Poll read_reg(0xE6004100) until value equals 0x12345678, with wait_on(5) between iterations. 21. Call finish(0).",
        "Test Steps / Procedure": "1. Initialize the system by writing to the control register. 2. Perform link training based on the configured device mode. 3. Configure cache coherency control registers for both ports by setting appropriate bit fields. 4. Wait for configuration to settle. 5. Repeat combined cache coherency bit field configuration for both ports. 6. Call non-secure protection configuration. 7. Poll the first SII status register until link readiness bits are set. 8. Poll the second SII status register until link readiness bits are set. 9. Read the vendor ID register and program memory base addresses for both ports. 10. Write enable values to subsystem control registers. 11. Disable cache by clearing coherency control bit fields for both ports. 12. Wait and confirm cache disable by re-clearing fields. 13. Program BAR registers for both PCIe slave ports with test patterns and verify readback. 14. Reprogram BAR registers with target address values and verify readback. 15. Poll completion status register until expected completion value is received. 16. Finish the test.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_HOST_PHY_IF_CFG; MIZAR_MIPI_DSI_HOST_PCKHDL_CFG; MIZAR_MIPI_DSI_HOST_CLKMGR_CFG; MIZAR_MIPI_DSI_SUBSYS_DPI_CONTROL; MIZAR_MIPI_DSI_DMAC_INTEN; MIZAR_MIPI_DSI_DMAC_DBGINST0; MIZAR_MIPI_DSI_DMAC_DBGINST1; MIZAR_MIPI_DSI_DMAC_DBGCMD; MIZAR_MIPI_DSI_DMAC_INTMIS; MIZAR_MIPI_DSI_DMAC_INTCLR",
        "Impacted Registers": "MIPI_DSI_HOST_PHY_IF_CFG; MIPI_DSI_HOST_PCKHDL_CFG; MIPI_DSI_HOST_CLKMGR_CFG; MIPI_DSI_SUBSYS_DPI_CONTROL; MIPI_DSI_DMAC_INTEN; MIPI_DSI_DMAC_DBGINST0; MIPI_DSI_DMAC_DBGINST1; MIPI_DSI_DMAC_DBGCMD; MIPI_DSI_DMAC_INTMIS; MIPI_DSI_DMAC_INTCLR",
        "Meta Validation / Acceptance Criteria": "The source code polls read_sii0_reg(0xC0) in a while loop until (data_rd & 0xD1) == 0xD1, and polls read_sii1_reg(0xC0) until the same condition is met. It then polls read_reg(0xE6004100) in a while loop with wait_on(5) between iterations until the read value equals 0x12345678. BAR register readbacks are performed after writing 0xFFFFFFFF and after reprogramming with target addresses, but no explicit comparison or assertion is present for those readback values. The test completes by calling finish(0), indicating a pass condition.",
        "Validation / Acceptance Criteria": "The test passes when the first and second SII status registers report link readiness with the expected status bits set. The completion status register must return the expected completion value. BAR register readback operations are performed after programming but without explicit value assertions in the source. The test calls the finish routine with a pass indicator upon successful completion of all polling conditions.",
        "Remarks": "The source code in the mipi_dsi_dbi_random_payload_test folder (program.c) contains PCIe link training and coherency control logic rather than MIPI DSI DBI random payload operations. The main.c file is an STM32 application with HAL initialization, motor control, and position sensing logic unrelated to MIPI DSI. The MIPI DSI register information in Meta Impacted Registers and Impacted Registers fields is derived from upstream Agent 2/3/4 outputs based on header-level analysis. There is a mismatch between the actual source code content and the MIPI DSI register context provided by upstream agents. Conditional compilation blocks (DM0_RC, DM1_RC, DM0_EP, DM1_EP) control link training and memory base programming paths. Multiple wait_on calls are used for timing synchronization. The MIPI_DSI_DMAC_INTMIS register is identified as a polled register by Agent 2."
    },
    {
        "Index": "3",
        "SS / Module": "MIPI_DSI",
        "Test Case Name": "mipi_dsi_subsys_reg_wr_rd_test",
        "Feature": "Subsystem Register Write Read Verification",
        "Meta Headers": "<stdlib.h>; <stdio.h>; <test_common.h>; \"pcie.h\"",
        "Meta Macros": "debug_print(...); LOGI(...); CTRL_REG_ADDR; SYNC_HANDSHAKE_VALUE; SII_LINK_STATUS_OFFSET; SII_LINK_UP_MASK; PCIE_SLV_VENDOR_ID_OFFSET; PCIE_SLV_CMD_STATUS_OFFSET; PCIE_CMD_MEM_IO_BUSMASTER; BAR0_REG_OFFSET; BAR1_REG_OFFSET; SEC_LAT_TIMER_OFFSET; SEC_STAT_IO_OFFSET; MEM_LIMIT_OFFSET; PREF_MEM_LIMIT_OFFSET; BAR_ENUM_PATTERN; BAR0_BASE_ADDR; BAR1_BASE_ADDR; BAR2_BASE_ADDR; BAR3_BASE_ADDR; BAR4_BASE_ADDR; BAR5_BASE_ADDR; SYS_REG_0; SYS_REG_1; SYS_REG_2; SYS_REG_3; SYS_REG_4; SYS_REG_5; TESTS_ITEM_DEFINED; TEST_OUTPUT_DEFINED",
        "Meta Arrays": "NA",
        "Speed": "NA",
        "Mode": "NA",
        "Memory Start Offset": "NA",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase validates MIPI DSI subsystem register write-read operations. According to Agent 2 analysis, the test iterates over an array of MIPI DSI subsystem register addresses (addr_array) containing MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL, MIZAR_MIPI_DSI_SUBSYS_LOW_PWR, MIZAR_MIPI_DSI_SUBSYS_DBITE, MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV, and MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW. The test performs two phases: (1) chk_rst_val() reads each register and compares the read value against a known default value using a read mask, and (2) chk_rd_wr() writes random data masked with a write mask to each register, then reads back and compares against the written value masked with a read mask. A skip_array controls which registers participate in the write-read phase; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW (skip_array[4]==1) is skipped during write-read but still read during reset value check. SOFT_RST_REG_ADDRESS is excluded per instructions. The program.c file in this folder contains PCIe initialization and link training code (write to 0xE6004100, conditional link_training_dm0_x4/dm1_x4, cache programming via SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF and SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF, polling read_sii0_reg and read_sii1_reg until link-up, memory base programming, BAR register enumeration, disable-cache programming, and polling 0xE6004100 for completion value 0x12345678). The test_define.c file contains PCIe-related macro definitions and extern function declarations.",
        "Test Description": "This test validates the write-read integrity of MIPI DSI subsystem registers. The test reads each subsystem register and verifies the reset default value against expected values using read masks. For writable registers, the test writes random data with appropriate write masks, reads back the register, and compares the result against the expected value using read masks. The test covers the Data FIFO Threshold Value, Low Power, DBITE, DBI FDIV, and Interrupt Raw registers in the MIPI DSI subsystem. The Interrupt Raw register is verified for reset value only and is skipped during the write-read phase.",
        "Meta Test Steps / Procedure": "1. System initialization: write 0x0 to control register at 0xE6004100. 2. Conditionally invoke link_training_dm0_x4(4) or link_training_dm1_x4(4) based on DM0_RC/DM1_RC/DM0_EP/DM1_EP compile-time defines. 3. Cache programming: read SOC_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF, modify bit fields [11:14], [3:6] with 0xF using set_data, write back. Repeat for bit fields [27:30], [19:22]. Repeat for SOC_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF. 4. wait_on(20), then repeat combined bit field modifications for both coherency control registers. 5. Poll read_sii0_reg(0xC0) until (data_rd & 0xD1) == 0xD1. 6. Call non_secure_prot_nic(). 7. Poll read_sii1_reg(0xC0) until (data_rd & 0xD1) == 0xD1. 8. Under DM0_RC: read vendor ID via read_pcie_slv0_reg(0x0), write command register, call mem_base_program_dm0_x4() and mem_base_program_dm1_x4(). 9. Write 0x1 to system registers at 0xE690000C, 0xE6900010, 0xE6900014, 0xE6900018, 0xE6900030, 0xE6900034. 10. Disable-cache programming: clear bit fields [27:30] and [19:22] to 0x0 in both coherency control registers. 11. wait_on(10), repeat disable-cache with combined fields. 12. wait_on(30), program BAR registers via write_pcie_slv1_reg and write_pcie_slv0_reg with 0xFFFFFFFF, read back, reprogram with target addresses, read back. 13. MIPI DSI subsystem register test phase (from Agent 2 analysis): chk_rst_val() iterates addr_array, for each register calls read_reg(addr), applies read mask, compares against default value. If mismatch, increments error counter err1. 14. chk_rd_wr() iterates addr_array, for each register where skip_array==0: generates random data_wr, applies write mask, calls write_reg(addr, data_wr), then calls read_reg(addr), applies read mask, compares read value against written value. If mismatch, increments error counter err2. MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW is skipped (skip_array[4]==1). 15. Poll read_reg(0xE6004100) until value equals 0x12345678 with wait_on(5) between iterations. 16. Call finish(0).",
        "Test Steps / Procedure": "1. Initialize the system by writing to the control register and performing link training based on the configured device mode. 2. Configure cache coherency control registers for both PCIe ports by setting appropriate bit fields. 3. Wait for configuration to settle and repeat combined cache coherency configuration. 4. Poll SII0 and SII1 status registers until link readiness bits are set. 5. Call non-secure protection configuration. 6. Program memory base addresses for both ports. 7. Write enable values to system-level configuration registers. 8. Disable cache by clearing coherency control bit fields for both ports. 9. Program BAR registers for both PCIe slave ports with test patterns and target addresses. 10. Read each MIPI DSI subsystem register and verify the reset default value matches the expected value. 11. For each writable MIPI DSI subsystem register (MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL, MIPI_DSI_SUBSYS_LOW_PWR, MIPI_DSI_SUBSYS_DBITE, MIPI_DSI_SUBSYS_DBI_FDIV), write random test data with appropriate write masks. 12. Read back each written register and verify the read value matches the expected written value using read masks. 13. Verify that the MIPI_DSI_SUBSYS_INTERRUPT_RAW register is read-only by confirming it is skipped during the write-read phase. 14. Poll the completion status register until the expected completion value is received. 15. Finish the test.",
        "Meta Impacted Registers": "MIZAR_MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL; MIZAR_MIPI_DSI_SUBSYS_LOW_PWR; MIZAR_MIPI_DSI_SUBSYS_DBITE; MIZAR_MIPI_DSI_SUBSYS_DBI_FDIV; MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Impacted Registers": "MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL; MIPI_DSI_SUBSYS_LOW_PWR; MIPI_DSI_SUBSYS_DBITE; MIPI_DSI_SUBSYS_DBI_FDIV; MIPI_DSI_SUBSYS_INTERRUPT_RAW",
        "Meta Validation / Acceptance Criteria": "In chk_rst_val(): for each register in addr_array, read_reg(addr) is called, the result is masked with the corresponding read_mask from rd_mask_array, and compared against the corresponding default value from default_val_array. If (read_value & read_mask) != default_value, error counter err1 is incremented. In chk_rd_wr(): for each register where skip_array==0, a random data_wr value is generated, masked with write_mask from wr_mask_array, written via write_reg(addr, data_wr), then read back via read_reg(addr), masked with read_mask, and compared against (data_wr & read_mask). If mismatch, error counter err2 is incremented. MIZAR_MIPI_DSI_SUBSYS_INTERRUPT_RAW (index 4, skip_array[4]==1) is only validated for reset default value, not for write-read. The overall test passes when err1 == 0 and err2 == 0. The program also polls 0xE6004100 until value equals 0x12345678 before calling finish(0).",
        "Validation / Acceptance Criteria": "The test passes when all MIPI DSI subsystem registers read back their expected reset default values after masking with read masks. For writable registers (MIPI_DSI_SUBSYS_DATA_FIFO_THRESHOLD_VAL, MIPI_DSI_SUBSYS_LOW_PWR, MIPI_DSI_SUBSYS_DBITE, MIPI_DSI_SUBSYS_DBI_FDIV), the written random data must match the read-back value after applying appropriate read masks. The MIPI_DSI_SUBSYS_INTERRUPT_RAW register must match its expected reset default value but is not subjected to write-read verification. Both error counters must remain zero for the test to pass. The completion status register must return the expected completion value before the test finishes.",
        "Remarks": "The program.c source code in this folder contains PCIe initialization, link training, cache coherency programming, BAR enumeration, and completion polling logic. The MIPI DSI subsystem register write-read test logic (chk_rst_val and chk_rd_wr functions operating on addr_array) was identified by upstream Agent 2 analysis from the original source. The test_define.c file is AI-generated and contains PCIe-related macro definitions rather than the original addr_array with MIPI DSI register macros. SOFT_RST_REG_ADDRESS is excluded per instructions. The skip_array mechanism ensures MIPI_DSI_SUBSYS_INTERRUPT_RAW is read-only validated. Conditional compilation blocks (DM0_RC, DM1_RC, DM0_EP, DM1_EP) control link training paths. Multiple wait_on calls are used for timing synchronization throughout the test."
    }
]

# TestPlan sheet columns
testplan_columns = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
    'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
    'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
    'Code Generation'
]

# MetaData sheet columns
metadata_columns = [
    'Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
    'Meta Headers', 'Meta Macros', 'Meta Arrays'
]

# Create workbook
wb = Workbook()

# --- TestPlan Sheet ---
ws_tp = wb.active
ws_tp.title = 'TestPlan'

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
for col_idx, col_name in enumerate(testplan_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

# Write TestPlan data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(testplan_columns, 1):
        value = row_data.get(col_name, '')
        if value is None:
            value = ''
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = cell_alignment
        cell.border = thin_border

# Freeze first row
ws_tp.freeze_panes = 'A2'

# Auto-size columns with max width cap
MAX_WIDTH = 60
for col_idx, col_name in enumerate(testplan_columns, 1):
    max_len = len(col_name)
    for row_idx in range(2, len(json_data) + 2):
        cell_value = str(ws_tp.cell(row=row_idx, column=col_idx).value or '')
        lines = cell_value.split('\n')
        for line in lines:
            if len(line) > max_len:
                max_len = len(line)
    adjusted_width = min(max_len + 2, MAX_WIDTH)
    ws_tp.column_dimensions[ws_tp.cell(row=1, column=col_idx).column_letter].width = adjusted_width

# --- MetaData Sheet ---
ws_md = wb.create_sheet('MetaData')

# Write MetaData headers
for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

# Write MetaData data
for row_idx, row_data in enumerate(json_data, 2):
    for col_idx, col_name in enumerate(metadata_columns, 1):
        value = row_data.get(col_name, '')
        if value is None:
            value = ''
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = cell_alignment
        cell.border = thin_border

# Freeze first row
ws_md.freeze_panes = 'A2'

# Auto-size MetaData columns
for col_idx, col_name in enumerate(metadata_columns, 1):
    max_len = len(col_name)
    for row_idx in range(2, len(json_data) + 2):
        cell_value = str(ws_md.cell(row=row_idx, column=col_idx).value or '')
        lines = cell_value.split('\n')
        for line in lines:
            if len(line) > max_len:
                max_len = len(line)
    adjusted_width = min(max_len + 2, MAX_WIDTH)
    ws_md.column_dimensions[ws_md.cell(row=1, column=col_idx).column_letter].width = adjusted_width

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# Save workbook
wb.save(output_path)
print(f'Workbook saved: {output_path}')
print(f'File size: {os.path.getsize(output_path)} bytes')

# Validation
try:
    wb_check = load_workbook(output_path)
    sheets = wb_check.sheetnames
    assert 'TestPlan' in sheets, 'TestPlan sheet missing'
    assert 'MetaData' in sheets, 'MetaData sheet missing'
    tp_rows = wb_check['TestPlan'].max_row - 1  # minus header
    md_rows = wb_check['MetaData'].max_row - 1
    print(f'Validation PASSED: TestPlan rows={tp_rows}, MetaData rows={md_rows}')
    print(f'MetaData sheet state: {wb_check["MetaData"].sheet_state}')
    wb_check.close()
except Exception as e:
    print(f'Validation FAILED: {e}')
    sys.exit(1)

# Output filename for workflow
with open(os.path.join(output_dir, 'generated_filename.txt'), 'w') as f:
    f.write(filename)

print(f'FILENAME={filename}')
print('SUCCESS')
