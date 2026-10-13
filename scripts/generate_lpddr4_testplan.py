#!/usr/bin/env python3
"""LPDDR4 TestPlan Excel Generator - Agent 7
Generates a real .xlsx workbook using openpyxl.
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

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"LPDDR4_TestPlan_{timestamp}.xlsx"

# Output directory
output_dir = os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))), "Test_Output", "Testing_Ag_7", "TestPlan")
os.makedirs(output_dir, exist_ok=True)
output_path = os.path.join(output_dir, filename)

# ============================================================
# TEST DATA
# ============================================================

testplan_columns = [
    "Index", "SS / Module", "Feature", "Test Case Name", "Test Description",
    "Speed", "Mode", "Memory Start Offset", "Memory End Offset", "Remarks",
    "Test Steps / Procedure", "Impacted Registers", "Validation / Acceptance Criteria",
    "Code Generation"
]

metadata_columns = [
    "Index", "Test Case Name", "Meta Test Description", "Meta Test Steps / Procedure",
    "Meta Impacted Registers", "Meta Validation / Acceptance Criteria",
    "Meta Headers", "Meta Macros", "Meta Arrays"
]

# Row 1
row1_tp = {
    "Index": "1",
    "SS / Module": "LPDDR4",
    "Feature": "LPDDR4 Data Mask (DM)",
    "Test Case Name": "lpddr4_mem_dm_test",
    "Test Description": "This test validates the LPDDR4 Data Mask (DM) functionality. The test configures the memory controller and PHY base addresses, sets full bus width mode, selects the speed grade, disables Data Bus Inversion, enables Data Mask, performs LPDDR4 memory training, then writes random 64-bit data patterns to memory via port0 and reads them back via both port0 and port1 to verify data integrity with Data Mask enabled.",
    "Speed": "Configurable via preprocessor: SG2667 \u2192 SG = 2667, SG2133 \u2192 SG = 2133, default \u2192 SG = 3200",
    "Mode": "bus_width = 0 (Full Bus)",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Source code retrieved from RAG is assembled from multiple fragments. The function int test_case() declares local variables: unsigned long int index; unsigned long int offset; unsigned long int exp_data_array[50]; unsigned long int read_data; unsigned int err0;. Global variables: unsigned long int port0_addr; unsigned long int port1_addr;. DM_EN = 1 is the distinguishing feature of this test (Data Mask enabled). DBI_EN = 0 (Data Bus Inversion disabled). The port0 write/read block is conditionally compiled using #if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^(defined(MPS_DRAM) && defined(AI_INITIATOR)) ^(defined(MPS_DRAM) && defined(DSP_INITIATOR))). All Agent 4 register mappings (APS_DRAM, SG2667, SG2133) are unresolved with register_name NA. The complete end-of-function return logic is not fully visible in the retrieved source context.",
    "Test Steps / Procedure": "1. Initialize GPV programming.\n2. Clear error counter (err0 = 0).\n3. Configure port addresses, controller base, and PHY base addresses (configuration varies based on APS_DRAM selection).\n4. Set bus width to Full Bus mode.\n5. Select speed grade (2667, 2133, or 3200 based on compile-time configuration).\n6. Disable Data Bus Inversion (DBI).\n7. Enable Data Mask (DM).\n8. Execute LPDDR4 memory training.\n9. Set memory access offset to 0x1000.\n10. Conditionally write 10 random 64-bit data values to port0 addresses using write_reg64 (guarded by initiator/DRAM type preprocessor condition).\n11. Read back data from port0 addresses using read_reg64 and compare against expected data array; increment error counter on mismatch.\n12. Read back data from port1 addresses using read_reg64 and compare against expected data array; increment error counter on mismatch.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "The test passes if the error counter (err0) remains at 0, indicating all 64-bit data values read back from both port0 and port1 match the expected data stored in exp_data_array. The test fails if any read-back data does not match the expected data, with each mismatch incrementing the error counter and printing the index, expected value, and actual value.",
    "Code Generation": ""
}

row1_md = {
    "Index": "1",
    "Test Case Name": "lpddr4_mem_dm_test",
    "Meta Test Description": "LPDDR4 Data Mask (DM) test. Function signature: int test_case(). Global variable declarations: unsigned long int port0_addr; unsigned long int port1_addr;. Local variable declarations: unsigned long int index; unsigned long int offset; unsigned long int exp_data_array[50]; unsigned long int read_data; unsigned int err0;. The test initializes GPV programming via gpv_programming(), sets err0 = 0, configures port addresses (port0_addr, port1_addr), controller base (ctl_base), and PHY base (phy_base) conditionally based on APS_DRAM define. Sets bus_width = 0 (Full Bus). Selects speed grade (SG) via conditional compilation: 2667 if SG2667 defined, 2133 if SG2133 defined, else 3200. Disables Data Bus Inversion (DBI_EN = 0). Enables Data Mask (DM_EN = 1). Calls lpddr4_training() to perform LPDDR4 memory training. After training, sets offset = 0x1000 and conditionally (guarded by XOR-based preprocessor condition on APS_DRAM/A53_INITIATOR, MPS_DRAM/AI_INITIATOR, MPS_DRAM/DSP_INITIATOR) writes 10 random 64-bit values to port0_addr using write_reg64(), then reads them back via read_reg64() comparing against exp_data_array[index]. Also performs port1 read-back verification loop of 10 iterations comparing read_data against exp_data_array[index]. Errors are tracked via err0 counter and reported via printf.",
    "Meta Test Steps / Procedure": "1. Call gpv_programming()\n2. err0 = 0\n3. Conditional on APS_DRAM:\n #if defined(APS_DRAM)\n port0_addr = 0\n port1_addr = 0x15A0000000\n ctl_base = 0x9EE03000\n phy_base = 0x9F000000\n #else\n port0_addr = 0\n port1_addr = 0x11A0000000\n ctl_base = 0x11D003000\n phy_base = 0x11D500000\n #endif\n4. bus_width = 0 //0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus\n5. Conditional on SG2667/SG2133:\n #if defined(SG2667)\n SG = 2667\n #elif defined(SG2133)\n SG = 2133\n #else\n SG = 3200\n #endif\n6. DBI_EN = 0\n7. DM_EN = 1\n8. Call lpddr4_training()\n9. offset = 0x1000\n10. #if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^(defined(MPS_DRAM) && defined(AI_INITIATOR)) ^(defined(MPS_DRAM) && defined(DSP_INITIATOR)))\n Port0 Write Loop:\n for(index = 0; index < 10; index++)\n {\n exp_data_array[index] = rand();\n exp_data_array[index] = exp_data_array[index] | ((unsigned long)rand() << 32);\n write_reg64(port0_addr + ((unsigned long)index * offset), exp_data_array[index]);\n }\n11. Port0 Read-back Verification Loop:\n for(index = 0; index < 10; index++)\n {\n read_reg64(port0_addr + ((unsigned long)indexoffset), &read_data);\n if(read_data != exp_data_array[index])\n {\n printf(\"ERROR_0: port0 index = %d, exp_data = %lx, actual_data = %lx\\n\", index, exp_data_array[index], read_data);\n err0++;\n }\n }\n #endif\n12. Port1 Read-back Verification Loop:\n for(index = 0; index < 10; index++)\n {\n read_reg64(port1_addr + ((unsigned long)index * offset), &read_data);\n if(read_data != exp_data_array[index])\n {\n printf(\"ERROR_0: port1 index = %d, exp_data = %lx, actual_data = %lx\\n\", index, exp_data_array[index], read_data);\n err0++;\n }\n }",
    "Meta Impacted Registers": "NA",
    "Meta Validation / Acceptance Criteria": "err0 is initialized to 0 at the start of the test.\nPort0 read-back validation: for(index = 0; index < 10; index++) { read_reg64(port0_addr + ((unsigned long)indexoffset), &read_data); if(read_data != exp_data_array[index]) { printf(\"ERROR_0: port0 index = %d, exp_data = %lx, actual_data = %lx\\n\", index, exp_data_array[index], read_data); err0++; } }\nPort1 read-back validation: for(index = 0; index < 10; index++) { read_reg64(port1_addr + ((unsigned long)index * offset), &read_data); if(read_data != exp_data_array[index]) { printf(\"ERROR_0: port1 index = %d, exp_data = %lx, actual_data = %lx\\n\", index, exp_data_array[index], read_data); err0++; } }\nPass condition: err0 == 0 (no mismatches detected across all read-back comparisons).\nFail condition: err0 != 0 (one or more read_data != exp_data_array[index] mismatches detected).",
    "Meta Headers": '#include <stdio.h>\n#include <stdlib.h>\n#include "test_common.h"\n#include "lpddr4.h"',
    "Meta Macros": "APS_DRAM (conditional compilation macro, controls port address and base address selection)\nSG2667 (conditional compilation macro, selects speed grade 2667)\nSG2133 (conditional compilation macro, selects speed grade 2133)\nA53_INITIATOR (conditional compilation macro, used in XOR-based port0 access guard)\nMPS_DRAM (conditional compilation macro, used in XOR-based port0 access guard)\nAI_INITIATOR (conditional compilation macro, used in XOR-based port0 access guard)\nDSP_INITIATOR (conditional compilation macro, used in XOR-based port0 access guard)",
    "Meta Arrays": "unsigned long int exp_data_array[50]; \u2014 Array of 50 elements of type unsigned long int. Used to store expected 64-bit random data values generated via rand() combined with ((unsigned long)rand() << 32). Populated in write loops for port0 (10 elements, offset 0x1000) and used for read-back comparison against read_data."
}

# Row 2
row2_tp = {
    "Index": "2",
    "SS / Module": "LPDDR4",
    "Feature": "LPDDR4 Inline ECC 1-bit Correctable Error Detection and Correction",
    "Test Case Name": "lpddr4_inline_ecc_1bit_data_modify_test",
    "Test Description": "This test validates LPDDR4 inline ECC 1-bit correctable error detection and correction during data modification. The test configures the memory controller and PHY, performs LPDDR4 training, initializes ECC configuration registers (ECCCFG0, ECCCFG1) with ecc_region_map = 0x55 (regions 0, 2, 4, 6 ECC-enabled), scrubs ECC-enabled regions via memory_init_scrb, writes random 64-bit data patterns to 7 memory regions via port1, enables ECC corrected error interrupts, then reads back data from each region to trigger ECC checking. For ECC-enabled regions, the test expects a correctable ECC interrupt to fire, which is handled by Default_IRQHandler. The handler verifies the ECCSTAT register for 1-bit error detection, clears the error via ECCCTL, and clears the interrupt. Data integrity is verified by comparing read-back data against expected values.",
    "Speed": "Configurable via preprocessor: SG2667 \u2192 SG = 2667, SG2133 \u2192 SG = 2133, default \u2192 SG = 3200",
    "Mode": "bus_width = 0 (Full Bus)",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Source code retrieved from RAG is assembled from multiple fragments. Function signatures: int test_case(); void Default_IRQHandler(); void memory_init_scrb(unsigned long int address);. Global variables: unsigned long int port0_addr; unsigned long int port1_addr; unsigned int int_expected; unsigned int eccstat; unsigned int count; unsigned int region; unsigned int ecc_region_map; unsigned int sbr_init; unsigned int err0;. Local variables in test_case(): unsigned long int offset; unsigned long int exp_data_array[50]; unsigned long int read_data; num_slverr_taken = 0;. Local variables in memory_init_scrb(): unsigned long int i; unsigned int rd_data;. ecc_region_map = 0x55 (binary 01010101) enables ECC for regions 0, 2, 4, 6. offset = 0x80000000 (2 GB region spacing). ECC_EN = 1, DM_EN = 1, DBI_EN = 0. The memory_init_scrb function body is only partially visible (sbr_init = 1, PCTRL_0 and PCTRL_1 disable); remaining scrubber register operations (SBRCTL, SBRWDATA0, SBRSTART, SBRSTAT) are not visible in retrieved source context. The Default_IRQHandler also writes to SYSREG_INTR_RSTCR_ADDR and calls GIC_ClearIRQ(CTL_INT_NO). All Agent 4 register mappings (APS_DRAM, SG2667, SG2133, SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK, SII_INTR_EN_SBR_DONE_INTR_MASK) are unresolved with register_name NA.",
    "Test Steps / Procedure": "1. Initialize GPV programming.\n2. Clear error counter (err0 = 0).\n3. Configure port addresses, controller base, and PHY base addresses (configuration varies based on APS_DRAM selection).\n4. Set bus width to Full Bus mode.\n5. Select speed grade (2667, 2133, or 3200 based on compile-time configuration).\n6. Disable Data Bus Inversion (DBI). Enable Data Mask (DM). Enable ECC.\n7. Execute LPDDR4 memory training.\n8. Wait for stabilization, signal training completion, wait again.\n9. Clear ECCCTL register.\n10. Enable SBR done interrupt via SII interrupt enable register.\n11. Set ecc_region_map = 0x55 (ECC enabled for regions 0, 2, 4, 6).\n12. Unlock SWCTL, configure ECCCFG0 with region map and mode, configure ECCCFG1, re-lock SWCTL.\n13. Scrub ECC-enabled memory regions via memory_init_scrb for each enabled region.\n14. Write random 64-bit data patterns to all 7 regions via port1.\n15. Enable ECC corrected error interrupt and SBR done interrupt via SII register.\n16. Enable corrected ECC error interrupt in ECCCTL (bit 8).\n17. For each of 7 regions: set int_expected based on ecc_region_map, read back data from port1, wait for interrupt propagation, verify data integrity against expected values.\n18. Call finish with error count to conclude the test.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "The test passes if the error counter (err0) is 0 when finish(err0) is called, indicating: (1) No unexpected interrupts were received during the test. (2) The ECCSTAT register correctly reported 1-bit correctable ECC errors (bit 8 set) for ECC-enabled regions. (3) The ECC corrected error interrupt was properly generated and cleared for ECC-enabled regions. (4) Data read back from all regions matches the expected data stored in exp_data_array. The test fails if any unexpected interrupts occur, if ECC error detection fails, or if data mismatches are detected.",
    "Code Generation": ""
}

row2_md = {
    "Index": "2",
    "Test Case Name": "lpddr4_inline_ecc_1bit_data_modify_test",
    "Meta Test Description": "LPDDR4 Inline ECC 1-bit correctable error data modify test. Function signatures: int test_case(); void Default_IRQHandler(); void memory_init_scrb(unsigned long int address);. Global variable declarations: unsigned long int port0_addr; unsigned long int port1_addr; unsigned int int_expected; unsigned int eccstat; unsigned int count; unsigned int region; unsigned int ecc_region_map; unsigned int sbr_init; unsigned int err0;. Local variable declarations in test_case(): unsigned long int offset; unsigned long int exp_data_array[50]; unsigned long int read_data; num_slverr_taken = 0;. Local variable declarations in memory_init_scrb(): unsigned long int i; unsigned int rd_data;. The test initializes GPV programming via gpv_programming(), sets err0 = 0, configures port addresses (port0_addr, port1_addr), controller base (ctl_base), and PHY base (phy_base) conditionally based on APS_DRAM define. Sets bus_width = 0 (Full Bus). Selects speed grade (SG) via conditional compilation. Sets DBI_EN = 0, DM_EN = 1, ECC_EN = 1. Calls lpddr4_training(). Calls wait_on(1000), training_done(), wait_on(1000). Writes 0x00000000 to UMCTL2_REGS.ECCCTL (ctl_base + 0x0000007c). Enables SBR done interrupt by writing SII_INTR_EN_SBR_DONE_INTR_MASK to ctl_base + 0x0000819C. Sets ecc_region_map = 0x55. Unlocks SWCTL by writing 0x00000000 to ctl_base + 0x00000320. Writes (ecc_region_map << 8) | 0xb4 to ECCCFG0 (ctl_base + 0x00000070). Writes 0x00000330 to ECCCFG1 (ctl_base + 0x00000074). Locks SWCTL by writing 0x00000001 to ctl_base + 0x00000320. Sets int_expected = 0, offset = 0x80000000. Iterates over 7 regions (region = 0 to 6), calling memory_init_scrb(offset*region) for each region where (ecc_region_map>>region)&0x1 is true. Writes random 64-bit data to each region: exp_data_array[region] = rand(); exp_data_array[region] = exp_data_array[region] | ((unsigned long)rand() << 32); write_reg64(port1_addr + (region * offset), exp_data_array[region]). Enables ECC corrected error and SBR done interrupts by writing SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK to ctl_base + 0x0000819C. Writes 1<<8 to UMCTL2_REGS.ECCCTL (ctl_base + 0x0000007c) to enable corrected ECC error interrupt. Iterates over 7 regions: sets int_expected = (ecc_region_map >> region) & 0x1, sets int_pend1 = 1, calls read_reg64(port1_addr + (region * offset), &read_data), calls wait_on(1000), and checks for expected interrupt and data comparison. Default_IRQHandler checks if interrupt is expected; if not, prints ERROR0 and increments err0; if expected, reads ECCSTAT (ctl_base + 0x00000078), checks bit 8 (eccstat & 0x100 == 0x100), clears via ECCCTL (ctl_base + 0x0000007c) with read_reg | 0x00000001, writes SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK to ctl_base + 0x00008194, prints Interrupt cleared, sets int_pend1 = 0, writes to SYSREG_INTR_RSTCR_ADDR, calls GIC_ClearIRQ(CTL_INT_NO). memory_init_scrb sets sbr_init = 1, disables ports by writing 0x00000000 to PCTRL_0 (ctl_base + 0x00000490) and PCTRL_1 (ctl_base + 0x00000540). Test concludes with finish(err0).",
    "Meta Test Steps / Procedure": "1. Call gpv_programming()\n2. err0 = 0\n3. Conditional on APS_DRAM:\n #if defined(APS_DRAM)\n port0_addr = 0\n port1_addr = 0x15A0000000\n ctl_base = 0x9EE03000\n phy_base = 0x9F000000\n #else\n port0_addr = 0\n port1_addr = 0x11A0000000\n ctl_base = 0x11D003000\n phy_base = 0x11D500000\n #endif\n4. bus_width = 0 //0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus\n5. Conditional on SG2667/SG2133:\n #if defined(SG2667)\n SG = 2667\n #elif defined(SG2133)\n SG = 2133\n #else\n SG = 3200\n #endif\n6. DBI_EN = 0\n7. DM_EN = 1\n8. ECC_EN = 1\n9. Call lpddr4_training()\n10. Call wait_on(1000)\n11. Call training_done()\n12. Call wait_on(1000)\n13. write_reg(ctl_base + 0x0000007c, 0x00000000) //UMCTL2_REGS.ECCCTL\n14. write_reg(ctl_base + 0x0000819C, SII_INTR_EN_SBR_DONE_INTR_MASK) //enabling intr\n15. ecc_region_map = 0x55\n16. write_reg(ctl_base + 0x00000320, 0x00000000) //UMCTL2_REGS.SWCTL\n17. write_reg(ctl_base + 0x00000070, (ecc_region_map << 8) | 0xb4) //UMCTL2_REGS.ECCCFG0\n18. write_reg(ctl_base + 0x00000074, 0x00000330) //UMCTL2_REGS.ECCCFG1\n19. write_reg(ctl_base + 0x00000320, 0x00000001) //UMCTL2_REGS.SWCTL\n20. int_expected = 0\n21. offset = 0x80000000\n22. for(region = 0; region < 7; region++) {\n if((ecc_region_map >> region) & 0x1) {\n memory_init_scrb(offset * region)\n }\n }\n23. for(region = 0; region < 7; region++) {\n exp_data_array[region] = rand()\n exp_data_array[region] = exp_data_array[region] | ((unsigned long)rand() << 32)\n write_reg64(port1_addr + (region * offset), exp_data_array[region])\n }\n24. write_reg(ctl_base + 0x0000819C, SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK) //enabling intr\n25. write_reg(ctl_base + 0x0000007c, 1 << 8) //UMCTL2_REGS.ECCCTL Interrupt enable\n26. for(region = 0; region < 7; region++) {\n int_expected = (ecc_region_map >> region) & 0x1\n int_pend1 = 1\n read_reg64(port1_addr + (region * offset), &read_data)\n wait_on(1000)\n // check for expected interrupt and data comparison\n if(exp_data_array[0] != read_data) {\n printf(\"ERROR region 0 no int: exp_data = %lx, actual_data = %lx\\n\", exp_data_array[0], read_data)\n err0++\n }\n }\n27. Call finish(err0)\n\nmemory_init_scrb(unsigned long int address):\n 1. sbr_init = 1\n 2. write_reg(ctl_base + 0x00000490, 0x00000000) //UMCTL2_MP.PCTRL_0\n 3. write_reg(ctl_base + 0x00000540, 0x00000000) //UMCTL2_MP.PCTRL_1\n (remaining steps not fully visible in retrieved source context)\n\nDefault_IRQHandler():\n 1. if(!int_expected) {\n printf(\"ERROR0: Unexpected interrupt\\n\")\n err0++\n int_pend = 1\n return\n }\n 2. eccstat = read_reg(ctl_base + 0x00000078) //UMCTL2_REGS.ECCSTAT\n 3. if((eccstat & 0x100) == 0x100) {\n write_reg(ctl_base + 0x0000007c, read_reg(ctl_base + 0x0000007c) | 0x00000001) //UMCTL2_REGS.ECCCTL\n write_reg(ctl_base + 0x00008194, SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK)\n printf(\"Interrupt cleared\")\n int_pend1 = 0\n }\n 4. write_reg(SYSREG_INTR_RSTCR_ADDR, SYSREG_CTRL_INTR_RSTCR_MASK)\n 5. GIC_ClearIRQ(CTL_INT_NO)",
    "Meta Impacted Registers": "NA",
    "Meta Validation / Acceptance Criteria": "Default_IRQHandler validation:\n if(!int_expected) { printf(\"ERROR0: Unexpected interrupt\\n\"); err0++; int_pend = 1; return; }\n eccstat = read_reg(ctl_base + 0x00000078); //UMCTL2_REGS.ECCSTAT\n if((eccstat & 0x100) == 0x100) { write_reg(ctl_base + 0x0000007c, read_reg(ctl_base + 0x0000007c) | 0x00000001); //UMCTL2_REGS.ECCCTL \u2014 clears correctable ECC error }\n\nECCSTAT polling validation:\n eccstat = read_reg(ctl_base + 0x00000078); //UMCTL2_REGS.ECCSTAT\n while(!(eccstat & (1 << 8))) { eccstat = read_reg(ctl_base + 0x00000078); //UMCTL2_REGS.ECCSTAT } \u2014 polls until bit 8 is set indicating 1-bit correctable ECC error detected\n\nData comparison validation:\n if(exp_data_array[0] != read_data) { printf(\"ERROR region 0 no int: exp_data = %lx, actual_data = %lx\\n\", exp_data_array[0], read_data); err0++; }\n\nPass condition: err0 == 0 passed to finish(err0).\nFail condition: err0 != 0 (unexpected interrupts, data mismatches, or ECC detection failures).",
    "Meta Headers": '#include <stdio.h>\n#include <stdlib.h>\n#include "test_common.h"\n#include "mps_sysreg.h"\n#include "aps_sysreg.h"\n#include "lpddr4.h"\n#include "lpddr4_aps_sii.h"\n#include "lpddr4_sii.h"',
    "Meta Macros": "#define SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK LPDDR4_SII_DDR_CNTRL_INTR_EN_DDR_ECC_CORRECTED_ERR_INTR_FAULT\n#define SII_INTR_EN_SBR_DONE_INTR_MASK LPDDR4_SII_DDR_CNTRL_INTR_EN_SBR_DONE_INTR\nAPS_DRAM (conditional compilation macro, controls port address and base address selection)\nSG2667 (conditional compilation macro, selects speed grade 2667)\nSG2133 (conditional compilation macro, selects speed grade 2133)\nA53_INITIATOR (conditional compilation macro, used in port access guard)\nMPS_DRAM (conditional compilation macro, used in port access guard)\nAI_INITIATOR (conditional compilation macro, used in port access guard)\nDSP_INITIATOR (conditional compilation macro, used in port access guard)",
    "Meta Arrays": "unsigned long int exp_data_array[50]; \u2014 Array of 50 elements of type unsigned long int. Used to store expected 64-bit random data values generated via rand() combined with ((unsigned long)rand() << 32). Populated in a loop for 7 regions (region = 0 to 6) and used for read-back comparison against read_data."
}

# Row 3
row3_tp = {
    "Index": "3",
    "SS / Module": "LPDDR4",
    "Feature": "LPDDR4 Inline ECC 2-bit Uncorrectable Error Detection",
    "Test Case Name": "lpddr4_inline_ecc_2bit_data_modify_test",
    "Test Description": "This test validates LPDDR4 inline ECC 2-bit uncorrectable error detection during data modification. The test configures the memory controller and PHY, performs LPDDR4 training, initializes ECC configuration registers (ECCCFG0, ECCCFG1) with ecc_region_map = 0x55 (regions 0, 2, 4, 6 ECC-enabled), scrubs ECC-enabled regions via memory_init_scrb, writes random 64-bit data patterns to 7 memory regions via port1, enables ECC uncorrected error interrupts, then reads back data from each region to trigger ECC checking. For ECC-enabled regions, the test expects an uncorrectable ECC interrupt to fire, which is handled by Default_IRQHandler. The handler verifies the ECCSTAT register for 2-bit error detection (bit 16), clears the error via ECCCTL (bit 1), and clears the interrupt. The test also tracks slave errors via slverr_taken and num_slverr_taken variables.",
    "Speed": "Configurable via preprocessor: SG2667 \u2192 SG = 2667, SG2133 \u2192 SG = 2133, default \u2192 SG = 3200",
    "Mode": "bus_width = 0 (Full Bus)",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Source code retrieved from RAG is assembled from multiple fragments. Function signatures: int test_case(); void Default_IRQHandler(); void memory_init_scrb(unsigned long int address);. Global variables: unsigned long int port0_addr; unsigned long int port1_addr; unsigned int int_expected; unsigned int eccstat; unsigned int count; unsigned int region; unsigned int ecc_region_map; unsigned int sbr_init; unsigned int err0; unsigned int slverr_taken; unsigned int num_slverr_taken;. Local variables in test_case(): unsigned long int offset; unsigned long int exp_data_array[50]; unsigned long int read_data; num_slverr_taken = 0;. Local variables in memory_init_scrb(): unsigned long int i; unsigned int rd_data;. ecc_region_map = 0x55 (binary 01010101) enables ECC for regions 0, 2, 4, 6. offset = 0x80000000 (2 GB region spacing). ECC_EN = 1, DM_EN = 1, DBI_EN = 0. Key differences from 1-bit ECC test: ECCCTL interrupt enable uses 1<<9 (vs 1<<8 for 1-bit), ECCSTAT checks bit 16 (0x10000) (vs bit 8 (0x100) for 1-bit), ECCCTL clear uses 0x00000002 (bit 1) (vs 0x00000001 (bit 0) for 1-bit), uses SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK (vs SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK for 1-bit), and includes slverr_taken/num_slverr_taken tracking not present in 1-bit test. The memory_init_scrb function body is only partially visible (sbr_init = 1, PCTRL_0 and PCTRL_1 disable); remaining scrubber register operations are not visible in retrieved source context. The macro SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK is defined as LPDDR4_SII_DDR_CNTRL_INTR_EN_DDR_ECC_UNCORRECTED_ERR_INTR_FAULT in the MPS (#else) branch; the APS branch definition is not directly visible in retrieved source context. All Agent 4 register mappings (APS_DRAM, SG2667, SG2133, SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK, SII_INTR_EN_SBR_DONE_INTR_MASK) are unresolved with register_name NA.",
    "Test Steps / Procedure": "1. Initialize GPV programming.\n2. Clear error counter (err0 = 0).\n3. Configure port addresses, controller base, and PHY base addresses (configuration varies based on APS_DRAM selection).\n4. Set bus width to Full Bus mode.\n5. Select speed grade (2667, 2133, or 3200 based on compile-time configuration).\n6. Disable Data Bus Inversion (DBI). Enable Data Mask (DM). Enable ECC.\n7. Execute LPDDR4 memory training.\n8. Wait for stabilization, signal training completion, wait again.\n9. Clear ECCCTL register.\n10. Enable SBR done interrupt via SII interrupt enable register.\n11. Set ecc_region_map = 0x55 (ECC enabled for regions 0, 2, 4, 6).\n12. Unlock SWCTL, configure ECCCFG0 with region map and mode, configure ECCCFG1, re-lock SWCTL.\n13. Scrub ECC-enabled memory regions via memory_init_scrb for each enabled region.\n14. Write random 64-bit data patterns to all 7 regions via port1.\n15. Enable ECC uncorrected error interrupt and SBR done interrupt via SII register.\n16. Enable uncorrected ECC error interrupt in ECCCTL (bit 9).\n17. For each of 7 regions: clear slverr_taken, print region number, set int_expected based on ecc_region_map, read back data from port1 to trigger ECC check.\n18. Poll ECCSTAT for bit 16 (uncorrectable error detection).\n19. Default_IRQHandler handles interrupt: verifies ECCSTAT bit 16, clears uncorrectable error via ECCCTL bit 1, clears SII interrupt, clears GIC interrupt.\n20. Call finish with error count to conclude the test.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "The test passes if the error counter (err0) is 0 when finish(err0) is called, indicating: (1) No unexpected interrupts were received during the test. (2) The ECCSTAT register correctly reported 2-bit uncorrectable ECC errors (bit 16 set) for ECC-enabled regions. (3) The ECC uncorrected error interrupt was properly generated and cleared for ECC-enabled regions via ECCCTL bit 1. (4) Slave error responses were properly tracked via slverr_taken. The test fails if any unexpected interrupts occur, if ECCSTAT bit 16 is not set when expected, or if the interrupt handler detects an unexpected interrupt source.",
    "Code Generation": ""
}

row3_md = {
    "Index": "3",
    "Test Case Name": "lpddr4_inline_ecc_2bit_data_modify_test",
    "Meta Test Description": "LPDDR4 Inline ECC 2-bit uncorrectable error data modify test. Function signatures: int test_case(); void Default_IRQHandler(); void memory_init_scrb(unsigned long int address);. Global variable declarations: unsigned long int port0_addr; unsigned long int port1_addr; unsigned int int_expected; unsigned int eccstat; unsigned int count; unsigned int region; unsigned int ecc_region_map; unsigned int sbr_init; unsigned int err0; unsigned int slverr_taken; unsigned int num_slverr_taken;. Local variable declarations in test_case(): unsigned long int offset; unsigned long int exp_data_array[50]; unsigned long int read_data; num_slverr_taken = 0;. Local variable declarations in memory_init_scrb(): unsigned long int i; unsigned int rd_data;. The test initializes GPV programming via gpv_programming(), sets err0 = 0, configures port addresses (port0_addr, port1_addr), controller base (ctl_base), and PHY base (phy_base) conditionally based on APS_DRAM define. Sets bus_width = 0 (Full Bus). Selects speed grade (SG) via conditional compilation. Sets DBI_EN = 0, DM_EN = 1, ECC_EN = 1. Calls lpddr4_training(). Calls wait_on(1000), training_done(), wait_on(1000). Writes 0x00000000 to UMCTL2_REGS.ECCCTL (ctl_base + 0x0000007c). Enables SBR done interrupt by writing SII_INTR_EN_SBR_DONE_INTR_MASK to ctl_base + 0x0000819C. Sets ecc_region_map = 0x55. Unlocks SWCTL by writing 0x00000000 to ctl_base + 0x00000320. Writes (ecc_region_map << 8) | 0xb4 to ECCCFG0 (ctl_base + 0x00000070). Writes 0x00000330 to ECCCFG1 (ctl_base + 0x00000074). Locks SWCTL by writing 0x00000001 to ctl_base + 0x00000320. Sets int_expected = 0, offset = 0x80000000. Iterates over 7 regions (region = 0 to 6), calling memory_init_scrb(offset*region) for each region where (ecc_region_map>>region)&0x1 is true. Writes random 64-bit data to each region: exp_data_array[region] = rand(); exp_data_array[region] = exp_data_array[region] | ((unsigned long)rand() << 32); write_reg64(port1_addr + (region * offset), exp_data_array[region]). Enables ECC uncorrected error and SBR done interrupts by writing SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK to ctl_base + 0x0000819C. Writes 1<<9 to UMCTL2_REGS.ECCCTL (ctl_base + 0x0000007c) to enable uncorrected ECC error interrupt. Iterates over 7 regions: sets slverr_taken = 0, prints region number via printf(\"region = %d\\n\", region), sets int_expected = (ecc_region_map >> region) & 0x1, sets int_pend1 = 1, calls read_reg64(port1_addr + (region * offset), &read_data). Also performs ECCSTAT polling: read_reg64(port1_addr, &read_data); eccstat = read_reg(ctl_base + 0x00000078); while(!(eccstat & (1 << 16))) { eccstat = read_reg(ctl_base + 0x00000078); }. Default_IRQHandler checks if interrupt is expected; if not, prints ERROR0 and increments err0; if expected, reads ECCSTAT (ctl_base + 0x00000078), checks bit 16 (eccstat & 0x10000 == 0x10000), clears via ECCCTL (ctl_base + 0x0000007c) with read_reg | 0x00000002, writes SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK to ctl_base + 0x00008194, prints Interrupt cleared, sets int_pend1 = 0; else prints ERROR: Unexpected interrupt and increments err0. Writes to SYSREG_INTR_RSTCR_ADDR with SYSREG_CTRL_INTR_RSTCR_MASK. Calls GIC_ClearIRQ(CTL_INT_NO). memory_init_scrb sets sbr_init = 1, disables ports by writing 0x00000000 to PCTRL_0 (ctl_base + 0x00000490) and PCTRL_1 (ctl_base + 0x00000540). Test concludes with finish(err0).",
    "Meta Test Steps / Procedure": "1. Call gpv_programming()\n2. err0 = 0\n3. Conditional on APS_DRAM:\n #if defined(APS_DRAM)\n port0_addr = 0\n port1_addr = 0x15A0000000\n ctl_base = 0x9EE03000\n phy_base = 0x9F000000\n #else\n port0_addr = 0\n port1_addr = 0x11A0000000\n ctl_base = 0x11D003000\n phy_base = 0x11D500000\n #endif\n4. bus_width = 0 //0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus\n5. Conditional on SG2667/SG2133:\n #if defined(SG2667)\n SG = 2667\n #elif defined(SG2133)\n SG = 2133\n #else\n SG = 3200\n #endif\n6. DBI_EN = 0\n7. DM_EN = 1\n8. ECC_EN = 1\n9. Call lpddr4_training()\n10. Call wait_on(1000)\n11. Call training_done()\n12. Call wait_on(1000)\n13. write_reg(ctl_base + 0x0000007c, 0x00000000) //UMCTL2_REGS.ECCCTL\n14. write_reg(ctl_base + 0x0000819C, SII_INTR_EN_SBR_DONE_INTR_MASK) //enabling intr\n15. ecc_region_map = 0x55\n16. write_reg(ctl_base + 0x00000320, 0x00000000) //UMCTL2_REGS.SWCTL\n17. write_reg(ctl_base + 0x00000070, (ecc_region_map << 8) | 0xb4) //UMCTL2_REGS.ECCCFG0\n18. write_reg(ctl_base + 0x00000074, 0x00000330) //UMCTL2_REGS.ECCCFG1\n19. write_reg(ctl_base + 0x00000320, 0x00000001) //UMCTL2_REGS.SWCTL\n20. int_expected = 0\n21. offset = 0x80000000\n22. for(region = 0; region < 7; region++) {\n if((ecc_region_map >> region) & 0x1) {\n memory_init_scrb(offset * region)\n }\n }\n23. for(region = 0; region < 7; region++) {\n exp_data_array[region] = rand()\n exp_data_array[region] = exp_data_array[region] | ((unsigned long)rand() << 32)\n write_reg64(port1_addr + (region * offset), exp_data_array[region])\n }\n24. write_reg(ctl_base + 0x0000819C, SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK) //enabling intr\n25. write_reg(ctl_base + 0x0000007c, 1 << 9) //UMCTL2_REGS.ECCCTL Interrupt enable\n26. for(region = 0; region < 7; region++) {\n slverr_taken = 0\n printf(\"region = %d\\n\", region)\n int_expected = (ecc_region_map >> region) & 0x1\n int_pend1 = 1\n read_reg64(port1_addr + (region * offset), &read_data)\n }\n27. ECCSTAT polling (source-visible fragment):\n read_reg64(port1_addr, &read_data)\n eccstat = read_reg(ctl_base + 0x00000078) //UMCTL2_REGS.ECCSTAT\n while(!(eccstat & (1 << 16))) {\n eccstat = read_reg(ctl_base + 0x00000078) //UMCTL2_REGS.ECCSTAT\n }\n28. Call finish(err0)\n\nmemory_init_scrb(unsigned long int address):\n 1. sbr_init = 1\n 2. //step 1: ECCCFG1.ecc_region_parity_lock to 1\n //Programmed before task call\n 3. //step 2:\n write_reg(ctl_base + 0x00000490, 0x00000000) //UMCTL2_MP.PCTRL_0\n write_reg(ctl_base + 0x00000540, 0x00000000) //UMCTL2_MP.PCTRL_1\n (remaining steps not fully visible in retrieved source context)\n\nDefault_IRQHandler():\n 1. if(!int_expected) {\n printf(\"ERROR0: Unexpected interrupt\\n\")\n err0++\n int_pend = 1\n return\n }\n 2. eccstat = read_reg(ctl_base + 0x00000078) //UMCTL2_REGS.ECCSTAT\n 3. if((eccstat & 0x10000) == 0x10000) {\n write_reg(ctl_base + 0x0000007c, read_reg(ctl_base + 0x0000007c) | 0x00000002) //UMCTL2_REGS.ECCCTL\n write_reg(ctl_base + 0x00008194, SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK)\n printf(\"Interrupt cleared\\n\")\n int_pend1 = 0\n } else {\n printf(\"ERROR: Unexpected interrupt\\n\")\n err0++\n }\n 4. write_reg(SYSREG_INTR_RSTCR_ADDR, SYSREG_CTRL_INTR_RSTCR_MASK)\n 5. GIC_ClearIRQ(CTL_INT_NO)",
    "Meta Impacted Registers": "NA",
    "Meta Validation / Acceptance Criteria": "Default_IRQHandler validation:\n if(!int_expected) { printf(\"ERROR0: Unexpected interrupt\\n\"); err0++; int_pend = 1; return; }\n eccstat = read_reg(ctl_base + 0x00000078); //UMCTL2_REGS.ECCSTAT\n if((eccstat & 0x10000) == 0x10000) {\n write_reg(ctl_base + 0x0000007c, read_reg(ctl_base + 0x0000007c) | 0x00000002); //UMCTL2_REGS.ECCCTL \u2014 clears uncorrectable ECC error\n write_reg(ctl_base + 0x00008194, SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK);\n printf(\"Interrupt cleared\\n\");\n int_pend1 = 0;\n } else {\n printf(\"ERROR: Unexpected interrupt\\n\");\n err0++;\n }\n\nECCSTAT polling validation:\n eccstat = read_reg(ctl_base + 0x00000078); //UMCTL2_REGS.ECCSTAT\n while(!(eccstat & (1 << 16))) { eccstat = read_reg(ctl_base + 0x00000078); //UMCTL2_REGS.ECCSTAT } \u2014 polls until bit 16 is set indicating 2-bit uncorrectable ECC error detected\n\nPass condition: err0 == 0 passed to finish(err0).\nFail condition: err0 != 0 (unexpected interrupts, ECCSTAT bit 16 not set when expected, or other error conditions).",
    "Meta Headers": '#include <stdio.h>\n#include <stdlib.h>\n#include "test_common.h"\n#include "mps_sysreg.h"\n#include "aps_sysreg.h"\n#include "lpddr4.h"\n#include "lpddr4_aps_sii.h"\n#include "lpddr4_sii.h"',
    "Meta Macros": "#define SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK LPDDR4_SII_DDR_CNTRL_INTR_EN_DDR_ECC_UNCORRECTED_ERR_INTR_FAULT\n#define SII_INTR_EN_SBR_DONE_INTR_MASK LPDDR4_SII_DDR_CNTRL_INTR_EN_SBR_DONE_INTR\nAPS_DRAM (conditional compilation macro, controls port address and base address selection, and selects APS-specific SII macro variants)\nSG2667 (conditional compilation macro, selects speed grade 2667)\nSG2133 (conditional compilation macro, selects speed grade 2133)\nA53_INITIATOR (conditional compilation macro, referenced in port access guard)\nMPS_DRAM (conditional compilation macro, referenced in port access guard)\nAI_INITIATOR (conditional compilation macro, referenced in port access guard)\nDSP_INITIATOR (conditional compilation macro, referenced in port access guard)",
    "Meta Arrays": "unsigned long int exp_data_array[50]; \u2014 Array of 50 elements of type unsigned long int. Used to store expected 64-bit random data values generated via rand() combined with ((unsigned long)rand() << 32). Populated in a loop for 7 regions (region = 0 to 6) and used for read-back comparison against read_data."
}

# Row 4
row4_tp = {
    "Index": "4",
    "SS / Module": "LPDDR4",
    "Feature": "LPDDR4 Half Data Bus Width",
    "Test Case Name": "lpddr4_mem_half_data_bus_width_test",
    "Test Description": "This test validates LPDDR4 memory operation in half data bus width mode. The test configures the memory controller and PHY base addresses, sets half bus width mode (bus_width = 1), selects the speed grade, disables Data Bus Inversion and Data Mask, performs LPDDR4 memory training, waits for stabilization, then reads back data from both port0 and port1 at offsets 0x100 and 0x108 and verifies against expected data patterns 0x3333333333333333 and 0x2222222222222222.",
    "Speed": "Configurable via preprocessor: SG2667 \u2192 SG = 2667, SG2133 \u2192 SG = 2133, default \u2192 SG = 3200",
    "Mode": "bus_width = 1 (Half Bus)",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Source code retrieved from RAG is assembled from multiple fragments. Function signature: int test_case(). Global variables: unsigned long int port0_addr; unsigned long int port1_addr;. bus_width = 1 is the distinguishing feature of this test (Half Bus mode). DBI_EN = 0 (Data Bus Inversion disabled). DM_EN = 0 (Data Mask disabled). The write_reg64 calls that write the test patterns 0x3333333333333333 and 0x2222222222222222 to port0_addr and port1_addr at offsets 0x100 and 0x108 are not visible in the retrieved source context. The port0 read-back verification is conditionally compiled using #if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^(defined(MPS_DRAM) && defined(AI_INITIATOR)) ^(defined(MPS_DRAM) && defined(DSP_INITIATOR))). Local variable declarations for read_data0, read_data1, read_data2, read_data3 are not directly visible in the retrieved source context but are used in the function body. All Agent 4 register mappings (APS_DRAM, SG2667, SG2133) are unresolved with register_name NA. Test concludes with finish(err0).",
    "Test Steps / Procedure": "1. Initialize GPV programming.\n2. Clear error counter (err0 = 0).\n3. Configure port addresses, controller base, and PHY base addresses (configuration varies based on APS_DRAM selection).\n4. Set bus width to Half Bus mode (bus_width = 1).\n5. Select speed grade (2667, 2133, or 3200 based on compile-time configuration).\n6. Disable Data Bus Inversion (DBI).\n7. Disable Data Mask (DM).\n8. Execute LPDDR4 memory training.\n9. Signal training completion.\n10. Wait for stabilization (1000 cycles).\n11. Conditionally read back data from port0 at offsets 0x100 and 0x108 and verify against expected patterns 0x3333333333333333 and 0x2222222222222222 (guarded by initiator/DRAM type preprocessor condition).\n12. Read back data from port1 at offsets 0x100 and 0x108 and verify against expected patterns 0x3333333333333333 and 0x2222222222222222.\n13. Call finish with error count to conclude the test.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "The test passes if the error counter (err0) is 0 when finish(err0) is called, indicating all data read back from port0 and port1 at offsets 0x100 and 0x108 matches the expected patterns 0x3333333333333333 and 0x2222222222222222 respectively. The test fails if both read values at a given port simultaneously mismatch their expected patterns, incrementing the error counter. The error condition uses logical AND (&&), so a single-value mismatch alone does not trigger an error.",
    "Code Generation": ""
}

row4_md = {
    "Index": "4",
    "Test Case Name": "lpddr4_mem_half_data_bus_width_test",
    "Meta Test Description": "LPDDR4 Half Data Bus Width test. Function signature: int test_case(). Global variable declarations: unsigned long int port0_addr; unsigned long int port1_addr;. The test initializes GPV programming via gpv_programming(), sets err0 = 0, configures port addresses (port0_addr, port1_addr), controller base (ctl_base), and PHY base (phy_base) conditionally based on APS_DRAM define. If defined(APS_DRAM): port0_addr = 0, port1_addr = 0x15A0000000, ctl_base = 0x9EE03000, phy_base = 0x9F000000. Else: port0_addr = 0, port1_addr = 0x11A0000000, ctl_base = 0x11D003000, phy_base = 0x11D500000. Sets bus_width = 1 (Half Bus). Selects speed grade (SG) via conditional compilation: 2667 if SG2667 defined, 2133 if SG2133 defined, else 3200. Sets DBI_EN = 0 (Data Bus Inversion disabled). Sets DM_EN = 0 (Data Mask disabled). Calls lpddr4_training() to perform LPDDR4 memory training. Calls training_done() to signal training completion. Calls wait_on(1000). Then conditionally (guarded by #if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^(defined(MPS_DRAM) && defined(AI_INITIATOR)) ^(defined(MPS_DRAM) && defined(DSP_INITIATOR)))) reads port0_addr + 0x100 into read_data0 and port0_addr + 0x108 into read_data1 via read_reg64(), and checks if((read_data0 != 0x3333333333333333) && (read_data1 != 0x2222222222222222)), printing error and incrementing err0 on mismatch. Then unconditionally reads port1_addr + 0x100 into read_data2 and port1_addr + 0x108 into read_data3 via read_reg64(), and checks if((read_data2 != 0x3333333333333333) && (read_data3 != 0x2222222222222222)), printing error and incrementing err0 on mismatch. Calls finish(err0) to conclude the test. The write_reg64 calls that write the 0x3333333333333333 and 0x2222222222222222 patterns are not visible in the retrieved source context.",
    "Meta Test Steps / Procedure": "1. Call gpv_programming()\n2. err0 = 0\n3. Conditional on APS_DRAM:\n #if defined(APS_DRAM)\n port0_addr = 0\n port1_addr = 0x15A0000000\n ctl_base = 0x9EE03000\n phy_base = 0x9F000000\n #else\n port0_addr = 0\n port1_addr = 0x11A0000000\n ctl_base = 0x11D003000\n phy_base = 0x11D500000\n #endif\n4. bus_width = 1 //0 -> Full Bus, 1 -> Half Bus, 2 -> Quarter bus\n5. Conditional on SG2667/SG2133:\n #if defined(SG2667)\n SG = 2667\n #elif defined(SG2133)\n SG = 2133\n #else\n SG = 3200\n #endif\n6. DBI_EN = 0\n7. DM_EN = 0\n8. Call lpddr4_training()\n9. Call training_done()\n10. Call wait_on(1000)\n11. #if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^(defined(MPS_DRAM) && defined(AI_INITIATOR)) ^(defined(MPS_DRAM) && defined(DSP_INITIATOR)))\n read_reg64(port0_addr + 0x100, &read_data0)\n read_reg64(port0_addr + 0x108, &read_data1)\n if((read_data0 != 0x3333333333333333) && (read_data1 != 0x2222222222222222))\n {\n printf(\"ERROR: port0 read_data0 = %lx, read_data1 = %lx\\n\", read_data0, read_data1)\n err0++\n }\n #endif\n12. read_reg64(port1_addr + 0x100, &read_data2)\n13. read_reg64(port1_addr + 0x108, &read_data3)\n14. if((read_data2 != 0x3333333333333333) && (read_data3 != 0x2222222222222222))\n {\n printf(\"ERROR: port1 read_data2 = %lx, read_data3 = %lx\\n\", read_data2, read_data3)\n err0++\n }\n15. Call finish(err0)\n(Note: write_reg64 calls that write the 0x3333333333333333 and 0x2222222222222222 patterns are not visible in the retrieved source context)",
    "Meta Impacted Registers": "NA",
    "Meta Validation / Acceptance Criteria": "Port0 read-back validation (conditional on #if !((defined(APS_DRAM) && defined(A53_INITIATOR)) ^(defined(MPS_DRAM) && defined(AI_INITIATOR)) ^(defined(MPS_DRAM) && defined(DSP_INITIATOR)))):\nread_reg64(port0_addr + 0x100, &read_data0)\nread_reg64(port0_addr + 0x108, &read_data1)\nif((read_data0 != 0x3333333333333333) && (read_data1 != 0x2222222222222222))\n{\n printf(\"ERROR: port0 read_data0 = %lx, read_data1 = %lx\\n\", read_data0, read_data1);\n err0++;\n}\n\nPort1 read-back validation (unconditional):\nread_reg64(port1_addr + 0x100, &read_data2)\nread_reg64(port1_addr + 0x108, &read_data3)\nif((read_data2 != 0x3333333333333333) && (read_data3 != 0x2222222222222222))\n{\n printf(\"ERROR: port1 read_data2 = %lx, read_data3 = %lx\\n\", read_data2, read_data3);\n err0++;\n}\n\nPass condition: err0 == 0 passed to finish(err0).\nFail condition: err0 != 0 (one or more read_data mismatches detected).\nNote: The error condition uses && (logical AND), meaning both read_data0 and read_data1 must mismatch their expected values simultaneously for err0 to be incremented for port0. Same logic applies for port1 with read_data2 and read_data3.",
    "Meta Headers": '#include <stdio.h>\n#include <stdlib.h>\n#include "test_common.h"\n#include "lpddr4.h"',
    "Meta Macros": "APS_DRAM (conditional compilation macro, controls port address and base address selection)\nSG2667 (conditional compilation macro, selects speed grade 2667)\nSG2133 (conditional compilation macro, selects speed grade 2133)\nA53_INITIATOR (conditional compilation macro, used in XOR-based port0 access guard)\nMPS_DRAM (conditional compilation macro, used in XOR-based port0 access guard)\nAI_INITIATOR (conditional compilation macro, used in XOR-based port0 access guard)\nDSP_INITIATOR (conditional compilation macro, used in XOR-based port0 access guard)",
    "Meta Arrays": "NA"
}

testplan_rows = [row1_tp, row2_tp, row3_tp, row4_tp]
metadata_rows = [row1_md, row2_md, row3_md, row4_md]

# ============================================================
# CREATE WORKBOOK
# ============================================================

wb = Workbook()

# --- TestPlan Sheet ---
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
for col_idx, col_name in enumerate(testplan_columns, 1):
    cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

# Write TestPlan data
for row_idx, row_data in enumerate(testplan_rows, 2):
    for col_idx, col_name in enumerate(testplan_columns, 1):
        value = row_data.get(col_name, "")
        cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = cell_alignment
        cell.border = thin_border

# --- MetaData Sheet ---
ws_md = wb.create_sheet(title="MetaData")

# Write MetaData headers
for col_idx, col_name in enumerate(metadata_columns, 1):
    cell = ws_md.cell(row=1, column=col_idx, value=col_name)
    cell.font = header_font
    cell.fill = header_fill
    cell.alignment = header_alignment
    cell.border = thin_border

# Write MetaData data
for row_idx, row_data in enumerate(metadata_rows, 2):
    for col_idx, col_name in enumerate(metadata_columns, 1):
        value = row_data.get(col_name, "")
        cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
        cell.alignment = cell_alignment
        cell.border = thin_border

# ============================================================
# FORMATTING
# ============================================================

# Freeze first row on both sheets
ws_tp.freeze_panes = 'A2'
ws_md.freeze_panes = 'A2'

# Auto-size columns with max width cap
MAX_WIDTH = 60
MIN_WIDTH = 12

def auto_size_columns(ws, max_w=MAX_WIDTH, min_w=MIN_WIDTH):
    for col in ws.columns:
        max_length = 0
        col_letter = col[0].column_letter
        for cell in col:
            try:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        if len(line) > max_length:
                            max_length = len(line)
            except:
                pass
        adjusted_width = min(max(max_length + 2, min_w), max_w)
        ws.column_dimensions[col_letter].width = adjusted_width

auto_size_columns(ws_tp)
auto_size_columns(ws_md)

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# ============================================================
# SAVE WORKBOOK
# ============================================================

wb.save(output_path)
print(f"WORKBOOK_SAVED:{output_path}")
print(f"FILENAME:{filename}")

# ============================================================
# VALIDATION
# ============================================================
import os
if os.path.exists(output_path):
    file_size = os.path.getsize(output_path)
    print(f"FILE_EXISTS:True")
    print(f"FILE_SIZE:{file_size}")
    
    # Reopen validation
    try:
        wb_check = load_workbook(output_path)
        sheets = wb_check.sheetnames
        print(f"SHEETS:{sheets}")
        print(f"TESTPLAN_ROWS:{ws_tp.max_row - 1}")
        print(f"METADATA_ROWS:{ws_md.max_row - 1}")
        print(f"VALIDATION:PASSED")
        wb_check.close()
    except Exception as e:
        print(f"VALIDATION:FAILED - {e}")
else:
    print(f"FILE_EXISTS:False")
    print(f"VALIDATION:FAILED")

wb.close()
