#!/usr/bin/env python3
"""LPDDR4 Test Plan Excel Generator - Agent 7
Generates a genuine Office Open XML (.xlsx) workbook using openpyxl.
Timestamp uses Indian Standard Time (IST, GMT+05:30).
"""
import json
import os
import sys
from datetime import datetime, timezone, timedelta

try:
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter
except ImportError:
    print("Installing openpyxl...")
    import subprocess
    subprocess.check_call([sys.executable, "-m", "pip", "install", "openpyxl"])
    from openpyxl import Workbook, load_workbook
    from openpyxl.styles import Font, PatternFill, Alignment, Border, Side
    from openpyxl.utils import get_column_letter

# ============================================================
# IST Timestamp
# ============================================================
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp_str = now_ist.strftime("%Y%m%d_%H%M%S")
IP_NAME = "LPDDR4"
FILENAME = f"{IP_NAME}_TestPlan_{timestamp_str}.xlsx"

# Output directory (same folder as this script)
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
OUTPUT_PATH = os.path.join(SCRIPT_DIR, FILENAME)

# ============================================================
# Input JSON Data (4 test cases)
# ============================================================
json_data = [
    {
        "Index": "1",
        "SS / Module": "LPDDR4",
        "Test Case Name": "lpddr4_inline_ecc_1bit_data_modify_test",
        "Feature": "Inline ECC 1-bit Corrected Error with Data Modify",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "mps_sysreg.h"; "aps_sysreg.h"; "lpddr4.h"; "lpddr4_aps_sii.h"; "lpddr4_sii.h"',
        "Meta Macros": "#define CTL_INT_NO APS_LPDDR4_CTL_INT_NO (APS_DRAM) / MPS_LPDDR4_CTL_INT_NO (else)\n#define PHY_INT_NO APS_LPDDR4_PHY_INT_NO (APS_DRAM) / MPS_LPDDR4_PHY_INT_NO (else)\n#define SYSREG_INTR_EN_ADDR MIZAR_APS_SYSREG_INTR_EN (APS_DRAM) / MIZAR_MPS_SYSREG_INTR_EN (else)\n#define SYSREG_CTRL_INTR_EN_MASK APS_SYSREG_INTR_EN_DDR_CTRL_INTR (APS_DRAM) / MPS_SYSREG_INTR_EN_DDR_CTRL_INTR (else)\n#define SYSREG_PHY_INTR_EN_MASK APS_SYSREG_INTR_EN_DDRPHY_DWC_DDRPHY_INT_N (APS_DRAM) / MPS_SYSREG_INTR_EN_DDRPHY_DWC_DDRPHY_INT_N (else)\n#define SYSREG_INTR_MSTS_ADDR MIZAR_APS_SYSREG_INTR_MSTS (APS_DRAM) / MIZAR_MPS_SYSREG_INTR_MSTS (else)\n#define SYSREG_CTRL_INTR_MSTS_MASK APS_SYSREG_INTR_MSTS_DDR_CTRL_INTR (APS_DRAM) / MPS_SYSREG_INTR_MSTS_DDR_CTRL_INTR (else)\n#define SYSREG_PHY_INTR_MSTS_MASK APS_SYSREG_INTR_MSTS_DDRPHY_DWC_DDRPHY_INT_N (APS_DRAM) / MPS_SYSREG_INTR_MSTS_DDRPHY_DWC_DDRPHY_INT_N (else)\n#define SYSREG_INTR_RSTCR_ADDR MIZAR_APS_SYSREG_INTR_RSTCR (APS_DRAM) / MIZAR_MPS_SYSREG_INTR_RSTCR (else)\n#define SYSREG_CTRL_INTR_RSTCR_MASK APS_SYSREG_INTR_RSTCR_DDR_CTRL_INTR (APS_DRAM) / MPS_SYSREG_INTR_RSTCR_DDR_CTRL_INTR (else)\n#define SYSREG_PHY_INTR_RSTCR_MASK APS_SYSREG_INTR_RSTCR_DDRPHY_DWC_DDRPHY_INT_N (APS_DRAM) / MPS_SYSREG_INTR_RSTCR_DDRPHY_DWC_DDRPHY_INT_N (else)\n#define SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK LPDDR4_APS_SII_DDR_CNTRL_INTR_EN_DDR_ECC_CORRECTED_ERR_INTR_FAULT (APS_DRAM) / LPDDR4_SII_DDR_CNTRL_INTR_EN_DDR_ECC_CORRECTED_ERR_INTR_FAULT (else)\n#define SII_INTR_EN_SBR_DONE_INTR_MASK LPDDR4_APS_SII_DDR_CNTRL_INTR_EN_SBR_DONE_INTR (APS_DRAM) / LPDDR4_SII_DDR_CNTRL_INTR_EN_SBR_DONE_INTR (else)",
        "Meta Arrays": "unsigned long int exp_data_array[50]; /* Populated at runtime via rand(). For region 0..6: exp_data_array[region] = rand(); exp_data_array[region] = exp_data_array[region] | ((unsigned long)rand() << 32); Total 7 elements populated (indices 0-6), array declared with size 50. */",
        "Speed": "SG3200 (default); SG2667 (if SG2667 defined); SG2133 (if SG2133 defined)",
        "Mode": "Full Bus (bus_width=0); DBI_EN=0; DM_EN=1; ECC_EN=1",
        "Memory Start Offset": "0x80000000 (region offset base, region*0x80000000 for scrub); port1_addr: 0x15A0000000 (APS_DRAM) / 0x11A0000000 (else)",
        "Memory End Offset": "address+0x1000 (scrub range end per region); region 6 end: 6*0x80000000 + 0x1000",
        "Meta Test Description": "This testcase validates LPDDR4 inline ECC 1-bit corrected error detection with data modification across multiple ECC regions. The test configures the DDR controller for inline ECC operation with ecc_region_map=0x55 (regions 0,2,4,6 enabled). It performs memory initialization via scrubber (SBR) for each enabled region, writes random 64-bit data to 7 regions via port1, triggers LPDDR4 training, then reads back data to verify ECC corrected-error interrupt behavior. For the first read (region 0), the test verifies that ECCSTAT bit[8] (ecc_corrected_err) is set and clears it. Then it enables the ECC corrected-error interrupt and iterates over all 7 regions: for ECC-enabled regions (0,2,4,6), it expects data match and an ECC corrected-error interrupt; for non-ECC regions (1,3,5), it expects data mismatch and no interrupt. Finally, it validates that ECCERRCNT equals 5 (the expected number of corrected errors: initial read + 4 enabled regions in the loop). The interrupt handler (Default_IRQHandler) handles both SBR-done interrupts during scrub initialization and ECC corrected-error interrupts during the read-back phase. The test uses the SYSREG interrupt enable, masked status, and reset-clear registers for interrupt management. Pass/fail is determined by the err0 error counter passed to finish().",
        "Test Description": "Verify LPDDR4 inline ECC 1-bit corrected error detection and interrupt generation across multiple ECC regions. The test configures ECC with a region map enabling regions 0, 2, 4, and 6, initializes memory via scrubber, writes random data to 7 regions, and reads back to verify that ECC-enabled regions produce corrected-error interrupts with correct data, while non-ECC regions produce data mismatches without interrupts. The final corrected error count is validated to equal 5.",
        "Meta Test Steps / Procedure": "1. Declare local variables: unsigned long int offset; unsigned long int exp_data_array[50]; unsigned long int read_data;\n2. Set int_pend = 1.\n3. Call GIC_EnableIRQ(CTL_INT_NO) to enable the DDR controller interrupt.\n4. write_reg(SYSREG_INTR_EN_ADDR, SYSREG_CTRL_INTR_EN_MASK) \u2014 enable DDR controller interrupt in SYSREG.\n5. Call gpv_programming() \u2014 external GPV programming.\n6. Set err0 = 0.\n7. Conditional platform initialization (APS_DRAM vs MPS).\n8. Set bus_width=0 (Full Bus).\n9. Conditional speed: SG2667 if SG2667 defined; SG2133 if SG2133 defined; else SG=3200.\n10. Set DBI_EN=0, DM_EN=1, ECC_EN=1.\n11. Call lpddr4_training().\n12-38. Full ECC test sequence with scrubber init, data write, training, read-back, interrupt validation, and ECCERRCNT check.",
        "Test Steps / Procedure": "1. Enable the DDR controller interrupt via GIC and configure the system register interrupt enable for DDR controller interrupts.\n2. Perform GPV programming.\n3. Configure platform-specific base addresses for controller, PHY, and memory ports.\n4. Set bus width to full, disable DBI, enable DM and ECC, and configure speed grade.\n5. Execute LPDDR4 training sequence.\n6. Enable SBR done interrupt in the SII interrupt enable register.\n7. Disable software programming lock (SWCTL), program ECCCFG0 with ecc_region_map=0x55 and ECCCFG1 with 0x330, then re-enable the lock.\n8. For each ECC-enabled region (0, 2, 4, 6), perform memory initialization via scrubber: disable ports, configure scrub mode and interval, set scrub data pattern to zero, program scrub start/range addresses, enable scrub, wait for SBR done interrupt, reconfigure scrub parameters, and re-enable ports.\n9. Write random 64-bit data to all 7 regions (0-6) via port 1.\n10. Wait, signal training done, and wait again.\n11. Clear ECC control register, read region 0 data, poll ECCSTAT for corrected error bit, clear the corrected error, and clear the SII interrupt status.\n12. Validate that region 0 read data matches expected data.\n13. Enable both ECC corrected error and SBR done interrupts, and enable ECC corrected error interrupt in ECCCTL.\n14. For each of the 7 regions: read data, and if the region is ECC-enabled, validate data match and wait for corrected-error interrupt; if not ECC-enabled, validate data mismatch.\n15. Read ECCERRCNT and validate the corrected error count equals 5.\n16. Call finish with the error counter to determine pass or fail.",
        "Meta Impacted Registers": "SYSREG_INTR_EN_ADDR; SYSREG_INTR_MSTS_ADDR; SYSREG_INTR_RSTCR_ADDR",
        "Impacted Registers": "NA",
        "Meta Validation / Acceptance Criteria": "1. After initial read of region 0: poll ECCSTAT until bit 8 is set.\n2. Validate exp_data_array[0] == read_data.\n3. For each ECC-enabled region: validate data match.\n4. For each ECC-enabled region: wait for corrected error interrupt.\n5. For each non-ECC region: validate data mismatch.\n6. Validate ECCERRCNT == 5.\n7-11. IRQ handler validations.\n11. Final pass/fail: finish(err0) \u2014 test passes if err0 == 0.",
        "Validation / Acceptance Criteria": "1. After reading region 0, the ECC status register must indicate a corrected error (bit 8 set).\n2. Data read from ECC-enabled regions (0, 2, 4, 6) must match the originally written random data.\n3. Data read from non-ECC regions (1, 3, 5) must NOT match the originally written data.\n4. Each read from an ECC-enabled region must trigger a corrected-error interrupt, which is validated and cleared in the interrupt handler.\n5. The corrected error count register must equal 5 after all region reads.\n6. The system register masked interrupt status must correctly identify DDR controller interrupts.\n7. During scrubber initialization, the SBR done interrupt must fire and the scrub busy status must clear.\n8. The test passes if no errors are accumulated (error counter equals zero).",
        "Remarks": "All three Agent 2 register-access macros (SYSREG_INTR_EN_ADDR, SYSREG_INTR_MSTS_ADDR, SYSREG_INTR_RSTCR_ADDR) are conditionally defined based on APS_DRAM vs MPS compile-time selection and were unresolved by Agent 3 and Agent 4. The testcase uses ctl_base as a runtime variable for DDR controller register access with hardcoded offsets. The ecc_region_map=0x55 enables ECC for regions 0, 2, 4, 6. The expected corrected error count of 5 corresponds to 1 initial read of region 0 plus 4 ECC-enabled regions in the loop."
    },
    {
        "Index": "2",
        "SS / Module": "LPDDR4",
        "Test Case Name": "lpddr4_inline_ecc_2bit_data_modify_test",
        "Feature": "Inline ECC 2-bit Uncorrected Error with Data Modify",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "mps_sysreg.h"; "aps_sysreg.h"; "lpddr4.h"; "lpddr4_aps_sii.h"; "lpddr4_sii.h"',
        "Meta Macros": "#define CTL_INT_NO APS_LPDDR4_CTL_INT_NO (if APS_DRAM) / MPS_LPDDR4_CTL_INT_NO (else)\n#define PHY_INT_NO APS_LPDDR4_PHY_INT_NO (if APS_DRAM) / MPS_LPDDR4_PHY_INT_NO (else)\n#define SYSREG_INTR_EN_ADDR MIZAR_APS_SYSREG_INTR_EN (if APS_DRAM) / MIZAR_MPS_SYSREG_INTR_EN (else)\n#define SYSREG_CTRL_INTR_EN_MASK APS_SYSREG_INTR_EN_DDR_CTRL_INTR (if APS_DRAM) / MPS_SYSREG_INTR_EN_DDR_CTRL_INTR (else)\n#define SYSREG_PHY_INTR_EN_MASK APS_SYSREG_INTR_EN_DDRPHY_DWC_DDRPHY_INT_N (if APS_DRAM) / MPS_SYSREG_INTR_EN_DDRPHY_DWC_DDRPHY_INT_N (else)\n#define SYSREG_INTR_MSTS_ADDR MIZAR_APS_SYSREG_INTR_MSTS (if APS_DRAM) / MIZAR_MPS_SYSREG_INTR_MSTS (else)\n#define SYSREG_CTRL_INTR_MSTS_MASK APS_SYSREG_INTR_MSTS_DDR_CTRL_INTR (if APS_DRAM) / MPS_SYSREG_INTR_MSTS_DDR_CTRL_INTR (else)\n#define SYSREG_PHY_INTR_MSTS_MASK APS_SYSREG_INTR_MSTS_DDRPHY_DWC_DDRPHY_INT_N (if APS_DRAM) / MPS_SYSREG_INTR_MSTS_DDRPHY_DWC_DDRPHY_INT_N (else)\n#define SYSREG_INTR_RSTCR_ADDR MIZAR_APS_SYSREG_INTR_RSTCR (if APS_DRAM) / MIZAR_MPS_SYSREG_INTR_RSTCR (else)\n#define SYSREG_CTRL_INTR_RSTCR_MASK APS_SYSREG_INTR_RSTCR_DDR_CTRL_INTR (if APS_DRAM) / MPS_SYSREG_INTR_RSTCR_DDR_CTRL_INTR (else)\n#define SYSREG_PHY_INTR_RSTCR_MASK APS_SYSREG_INTR_RSTCR_DDRPHY_DWC_DDRPHY_INT_N (if APS_DRAM) / MPS_SYSREG_INTR_RSTCR_DDRPHY_DWC_DDRPHY_INT_N (else)\n#define SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK LPDDR4_APS_SII_DDR_CNTRL_INTR_EN_DDR_ECC_UNCORRECTED_ERR_INTR_FAULT (if APS_DRAM) / LPDDR4_SII_DDR_CNTRL_INTR_EN_DDR_ECC_UNCORRECTED_ERR_INTR_FAULT (else)\n#define SII_INTR_EN_SBR_DONE_INTR_MASK LPDDR4_APS_SII_DDR_CNTRL_INTR_EN_SBR_DONE_INTR (if APS_DRAM) / LPDDR4_SII_DDR_CNTRL_INTR_EN_SBR_DONE_INTR (else)",
        "Meta Arrays": "unsigned long int exp_data_array[50]; /* Declared with size 50. Populated at runtime for indices 0-6. */",
        "Speed": "SG3200 (default); SG2667 (if SG2667 defined); SG2133 (if SG2133 defined)",
        "Mode": "Full Bus (bus_width=0); DBI_EN=0; DM_EN=1; ECC_EN=1",
        "Memory Start Offset": "0x80000000 (region offset base, region*0x80000000 for scrub); port1_addr: 0x15A0000000 (APS_DRAM) / 0x11A0000000 (else)",
        "Memory End Offset": "address+0x1000 (scrub range end per region); region 6 end: 6*0x80000000 + 0x1000",
        "Meta Test Description": "This testcase validates LPDDR4 inline ECC 2-bit uncorrected error detection with data modification across multiple ECC regions. The test configures the DDR controller for inline ECC operation with ecc_region_map=0x55 (regions 0,2,4,6 enabled). It performs memory initialization via scrubber (SBR) for each enabled region, writes random 64-bit data to 7 regions via port1, triggers LPDDR4 training, then reads back data to verify ECC uncorrected-error interrupt behavior. For the initial read (region 0), the test verifies that ECCSTAT bit[16] (ecc_uncorrected_err) is set and clears it via ECCCTL bit[1]. Then it enables the ECC uncorrected-error interrupt (ECCCTL bit[9]) and iterates over all 7 regions: for ECC-enabled regions (0,2,4,6), it expects an uncorrected-error interrupt and a slave error (slverr_taken); for all regions, data matching the original write is treated as an error. The test validates that the uncorrected error count (ECCERRCNT bits[31:16]) equals 5 and num_slverr_taken equals 5. Pass/fail is determined by the err0 error counter passed to finish().",
        "Test Description": "Verify LPDDR4 inline ECC 2-bit uncorrected error detection, interrupt generation, and slave error behavior across multiple ECC regions. The test configures ECC with a region map enabling regions 0, 2, 4, and 6, initializes memory via scrubber, writes random data to 7 regions, and reads back to verify that ECC-enabled regions produce uncorrected-error interrupts with slave errors, and that data does not match the original writes for any region. The final uncorrected error count and slave error count are both validated to equal 5.",
        "Meta Test Steps / Procedure": "1. Declare local variables.\n2. Set num_slverr_taken = 0.\n3. Set int_pend = 1.\n4-39. Full 2-bit ECC uncorrected error test sequence with scrubber init, data write, training, read-back, interrupt and slave error validation, and ECCERRCNT check.",
        "Test Steps / Procedure": "1. Enable the DDR controller interrupt via GIC and configure the system register interrupt enable for DDR controller interrupts.\n2. Perform GPV programming.\n3. Configure platform-specific base addresses for controller, PHY, and memory ports.\n4. Set bus width to full, disable DBI, enable DM and ECC, and configure speed grade.\n5. Execute LPDDR4 training sequence.\n6. Enable SBR done interrupt in the SII interrupt enable register.\n7. Disable software programming lock (SWCTL), program ECCCFG0 with ecc_region_map=0x55 and ECCCFG1 with 0x330, then re-enable the lock.\n8. For each ECC-enabled region (0, 2, 4, 6), perform memory initialization via scrubber.\n9. Write random 64-bit data to all 7 regions (0-6) via port 1.\n10. Wait, signal training done, and wait again.\n11. Clear ECC control register, read region 0 data, poll ECCSTAT for uncorrected error bit (bit 16), clear the uncorrected error via ECCCTL bit 1, and clear the SII interrupt status.\n12. Enable both ECC uncorrected error and SBR done interrupts in SII, and enable ECC uncorrected error interrupt in ECCCTL (bit 9).\n13. For each of the 7 regions: read data, and if the region is ECC-enabled, wait for uncorrected-error interrupt and verify a slave error was taken; for all regions, verify data does NOT match the originally written data.\n14. Read ECCERRCNT and validate the uncorrected error count (upper 16 bits) equals 5.\n15. Validate that the total number of slave errors taken equals 5.\n16. Call finish with the error counter to determine pass or fail.",
        "Meta Impacted Registers": "SYSREG_INTR_EN_ADDR; SYSREG_INTR_MSTS_ADDR; SYSREG_INTR_RSTCR_ADDR",
        "Impacted Registers": "NA",
        "Meta Validation / Acceptance Criteria": "1. Poll ECCSTAT until bit 16 is set.\n2. For each ECC-enabled region: wait for uncorrected error interrupt.\n3. For each ECC-enabled region: validate slverr_taken != 0.\n4. For ALL regions: validate data mismatch.\n5. Validate (ECCERRCNT>>16) == 5.\n6. Validate num_slverr_taken == 5.\n7-11. IRQ handler validations.\n11. Final pass/fail: finish(err0).",
        "Validation / Acceptance Criteria": "1. After reading region 0, the ECC status register must indicate an uncorrected error (bit 16 set).\n2. Each read from an ECC-enabled region (0, 2, 4, 6) must trigger an uncorrected-error interrupt, which is validated and cleared in the interrupt handler.\n3. Each read from an ECC-enabled region must produce a slave error.\n4. Data read from all regions (0-6) must NOT match the originally written random data.\n5. The uncorrected error count (upper 16 bits of ECCERRCNT) must equal 5 after all region reads.\n6. The total number of slave errors taken must equal 5.\n7. The system register masked interrupt status must correctly identify DDR controller interrupts.\n8. During scrubber initialization, the SBR done interrupt must fire and the scrub busy status must clear.\n9. The test passes if no errors are accumulated (error counter equals zero).",
        "Remarks": "All three Agent 2 register-access macros are conditionally defined based on APS_DRAM vs MPS compile-time selection and were unresolved by Agent 3 and Agent 4. This testcase differs from the 1-bit corrected error test by targeting 2-bit uncorrected ECC errors: it checks ECCSTAT bit[16] instead of bit[8], uses ECCCTL bit[1] for clearing instead of bit[0], uses ECCCTL bit[9] for interrupt enable instead of bit[8], and validates the uncorrected error count in ECCERRCNT bits[31:16]. Additionally, this test expects slave errors for ECC-enabled regions and validates that data does NOT match for ALL regions."
    },
    {
        "Index": "3",
        "SS / Module": "LPDDR4",
        "Test Case Name": "lpddr4_mem_dm_test",
        "Feature": "Memory Data Masking (DM)",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "lpddr4.h"',
        "Meta Macros": "NA",
        "Meta Arrays": "unsigned long int exp_data_array[50]; /* Declared with size 50. No static initializer. Populated at runtime in two phases. */",
        "Speed": "SG3200 (default); SG2667 (if SG2667 defined); SG2133 (if SG2133 defined)",
        "Mode": "Full Bus (bus_width=0); DBI_EN=0; DM_EN=1",
        "Memory Start Offset": "port0_addr: 0 (both APS_DRAM and else); port1_addr: 0x15A0000000 (APS_DRAM) / 0x11A0000000 (else)",
        "Memory End Offset": "Conditional block: port0_addr + 9*0x1000 = port0_addr + 0x9000; port1_addr + 9*0x1000 = port1_addr + 0x9000. Unconditional block: port1_addr + 31*0x20000000 = port1_addr + 0x3E0000000",
        "Meta Test Description": "This testcase validates LPDDR4 memory data masking (DM) functionality. DM_EN is set to 1 and DBI_EN is set to 0 with full bus width. The test performs LPDDR4 training, then executes two memory access phases. Phase 1 (conditional): writes 10 random 64-bit values to port0 at offset 0x1000 spacing, reads them back via port0 to validate, then reads the same addresses via port1 to validate cross-port data coherency. Phase 2 (unconditional): writes 32 random 64-bit values to port1 at offset 0x20000000 spacing, then reads them back via port1 to validate. Pass/fail is determined by the err0 error counter passed to finish().",
        "Test Description": "Verify LPDDR4 memory data masking (DM) functionality by writing random data patterns and reading them back. The test enables DM, performs training, writes random 64-bit data to memory via port 0 and port 1, reads back via both ports to verify data integrity and cross-port coherency, and validates all read data matches the expected written data.",
        "Meta Test Steps / Procedure": "1. Declare local variables.\n2. Declare global variables.\n3. Call gpv_programming().\n4. Set err0 = 0.\n5. Conditional platform initialization.\n6-18. Two-phase memory write/read test with cross-port validation.",
        "Test Steps / Procedure": "1. Perform GPV programming.\n2. Configure platform-specific base addresses for controller, PHY, and memory ports.\n3. Set bus width to full, disable DBI, enable DM, and configure speed grade.\n4. Execute LPDDR4 training sequence.\n5. (Conditional) Write 10 random 64-bit data values to port 0 at 0x1000 spacing.\n6. (Conditional) Read back all 10 values from port 0 and validate each matches the expected data.\n7. (Conditional) Read back all 10 values from port 1 (cross-port) and validate each matches the expected data.\n8. Write 32 random 64-bit data values to port 1 at 0x20000000 spacing.\n9. Read back all 32 values from port 1 and validate each matches the expected data.\n10. Call finish with the error counter to determine pass or fail.",
        "Meta Impacted Registers": "NA",
        "Impacted Registers": "NA",
        "Meta Validation / Acceptance Criteria": "1. Phase 1 port0 read-back: validate read_data == exp_data_array[index] for each index 0-9.\n2. Phase 1 port1 cross-port read-back: validate read_data == exp_data_array[index] for each index 0-9.\n3. Phase 2 port1 read-back: validate read_data == exp_data_array[index] for each index 0-31.\n4. Final pass/fail: finish(err0) \u2014 test passes if err0 == 0.",
        "Validation / Acceptance Criteria": "1. All 10 random data values written to port 0 must be correctly read back from port 0 (conditional phase).\n2. All 10 values written to port 0 must also be correctly read back from port 1, verifying cross-port data coherency (conditional phase).\n3. All 32 random data values written to port 1 at large address spacing must be correctly read back from port 1.\n4. The test passes if no data mismatches are detected (error counter equals zero).",
        "Remarks": "This is a pure memory read/write test with DM (Data Masking) enabled. No DDR controller register accesses are performed. The test has two phases: Phase 1 is conditionally compiled. Phase 2 is unconditional and uses a large offset of 0x20000000 (512 MB) between accesses."
    },
    {
        "Index": "4",
        "SS / Module": "LPDDR4",
        "Test Case Name": "lpddr4_mem_half_data_bus_width_test",
        "Feature": "Half Data Bus Width Memory Access",
        "Meta Headers": '<stdio.h>; <stdlib.h>; "test_common.h"; "lpddr4.h"',
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Speed": "SG3200 (default); SG2667 (if SG2667 defined); SG2133 (if SG2133 defined)",
        "Mode": "Half Bus (bus_width=1); DBI_EN=0; DM_EN=0",
        "Memory Start Offset": "port0_addr: 0 (both APS_DRAM and else); port1_addr: 0x15A0000000 (APS_DRAM) / 0x11A0000000 (else)",
        "Memory End Offset": "port0_addr + 0x108; port1_addr + 0x108",
        "Meta Test Description": "This testcase validates LPDDR4 memory access in half data bus width mode. bus_width is set to 1 (Half Bus), DBI_EN=0, DM_EN=0. The test performs LPDDR4 training, calls training_done(), waits 1000 cycles, then reads pre-existing data patterns from two memory locations at offsets 0x100 and 0x108. The validation condition uses logical AND (&&): an error is flagged only if BOTH read values do not match their respective expected patterns simultaneously. No data is written by this test. Pass/fail is determined by the err0 error counter passed to finish().",
        "Test Description": "Verify LPDDR4 memory read access in half data bus width mode. The test configures the controller for half bus width, performs training, and reads data from two memory locations via port 0 (conditional) and port 1 (unconditional) to verify that the expected fixed data patterns are present in memory after training.",
        "Meta Test Steps / Procedure": "1. Declare local variables.\n2. Declare global variables.\n3. Call gpv_programming().\n4-19. Half bus width read-only test with pattern validation.",
        "Test Steps / Procedure": "1. Perform GPV programming.\n2. Configure platform-specific base addresses for controller, PHY, and memory ports.\n3. Set bus width to half, disable DBI and DM, and configure speed grade.\n4. Execute LPDDR4 training sequence.\n5. Signal training done and wait for stabilization.\n6. (Conditional) Read two 64-bit values from port 0 at offsets 0x100 and 0x108, and validate against expected patterns.\n7. Read two 64-bit values from port 1 at offsets 0x100 and 0x108, and validate against expected patterns.\n8. Call finish with the error counter to determine pass or fail.",
        "Meta Impacted Registers": "NA",
        "Impacted Registers": "NA",
        "Meta Validation / Acceptance Criteria": "1. Phase 1 port0 read: validate using AND logic.\n2. Phase 2 port1 read: validate using AND logic.\n3. Final pass/fail: finish(err0) \u2014 test passes if err0 == 0.",
        "Validation / Acceptance Criteria": "1. Data read from port 0 at offset 0x100 must equal 0x3333333333333333 or data at offset 0x108 must equal 0x2222222222222222 (conditional phase).\n2. Data read from port 1 at offset 0x100 must equal 0x3333333333333333 or data at offset 0x108 must equal 0x2222222222222222.\n3. The test passes if no errors are accumulated (error counter equals zero).",
        "Remarks": "This is a read-only memory verification test in half data bus width mode (bus_width=1). No data is written by this test. DM is disabled (DM_EN=0). The validation uses logical AND (&&), meaning an error is only flagged if BOTH read values fail to match their expected patterns simultaneously. No DDR controller register accesses are performed."
    }
]

# ============================================================
# TestPlan Sheet Columns
# ============================================================
TP_COLUMNS = [
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
# MetaData Sheet Columns
# ============================================================
MD_COLUMNS = [
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

# ============================================================
# Formatting Constants
# ============================================================
HEADER_FONT = Font(name="Calibri", bold=True, color="FFFFFF", size=11)
HEADER_FILL = PatternFill(start_color="4472C4", end_color="4472C4", fill_type="solid")
HEADER_ALIGN = Alignment(horizontal="center", vertical="center", wrap_text=True)
CELL_ALIGN = Alignment(vertical="top", wrap_text=True)
THIN_BORDER = Border(
    left=Side(style="thin"),
    right=Side(style="thin"),
    top=Side(style="thin"),
    bottom=Side(style="thin")
)
MAX_COL_WIDTH = 60
MIN_COL_WIDTH = 12


def auto_size_columns(ws, max_width=MAX_COL_WIDTH, min_width=MIN_COL_WIDTH):
    """Auto-size columns based on content with min/max bounds."""
    for col_idx in range(1, ws.max_column + 1):
        max_len = 0
        col_letter = get_column_letter(col_idx)
        for row in ws.iter_rows(min_col=col_idx, max_col=col_idx, values_only=False):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split("\n")
                    for line in lines:
                        max_len = max(max_len, len(line))
        width = min(max(max_len + 2, min_width), max_width)
        ws.column_dimensions[col_letter].width = width


def format_sheet(ws):
    """Apply formatting to a worksheet."""
    # Format header row
    for cell in ws[1]:
        cell.font = HEADER_FONT
        cell.fill = HEADER_FILL
        cell.alignment = HEADER_ALIGN
        cell.border = THIN_BORDER

    # Format data cells
    for row in ws.iter_rows(min_row=2, max_row=ws.max_row, max_col=ws.max_column):
        for cell in row:
            cell.alignment = CELL_ALIGN
            cell.border = THIN_BORDER

    # Freeze first row
    ws.freeze_panes = "A2"

    # Auto-size columns
    auto_size_columns(ws)


def main():
    print(f"Generating workbook: {FILENAME}")
    print(f"IST Timestamp: {now_ist.strftime('%Y-%m-%d %H:%M:%S %Z')}")
    print(f"Output path: {OUTPUT_PATH}")

    # Create workbook
    wb = Workbook()

    # ---- TestPlan Sheet ----
    ws_tp = wb.active
    ws_tp.title = "TestPlan"

    # Write header
    ws_tp.append(TP_COLUMNS)

    # Write data rows
    for row_data in json_data:
        row = []
        for col in TP_COLUMNS:
            if col == "Code Generation":
                row.append("")  # Empty for Code Generation
            else:
                row.append(row_data.get(col, ""))
        ws_tp.append(row)

    format_sheet(ws_tp)
    print(f"TestPlan sheet: {ws_tp.max_row - 1} data rows")

    # ---- MetaData Sheet ----
    ws_md = wb.create_sheet(title="MetaData")

    # Write header
    ws_md.append(MD_COLUMNS)

    # Write data rows
    for row_data in json_data:
        row = []
        for col in MD_COLUMNS:
            row.append(row_data.get(col, ""))
        ws_md.append(row)

    format_sheet(ws_md)
    print(f"MetaData sheet: {ws_md.max_row - 1} data rows")

    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = "veryHidden"

    # ---- Save Workbook ----
    wb.save(OUTPUT_PATH)
    print(f"Workbook saved: {OUTPUT_PATH}")

    # ---- Post-Save Validation ----
    if not os.path.exists(OUTPUT_PATH):
        print("ERROR: File does not exist after save!")
        sys.exit(1)

    file_size = os.path.getsize(OUTPUT_PATH)
    if file_size == 0:
        print("ERROR: File size is 0!")
        sys.exit(1)
    print(f"File size: {file_size} bytes")

    # Reopen and validate
    wb_check = load_workbook(OUTPUT_PATH)
    sheet_names = wb_check.sheetnames
    print(f"Sheet names: {sheet_names}")

    assert "TestPlan" in sheet_names, "TestPlan sheet missing!"
    assert "MetaData" in sheet_names, "MetaData sheet missing!"
    assert wb_check["MetaData"].sheet_state == "veryHidden", "MetaData not veryHidden!"
    assert wb_check["TestPlan"].max_row == 5, f"Expected 5 rows (1 header + 4 data), got {wb_check['TestPlan'].max_row}"
    assert wb_check["MetaData"].max_row == 5, f"Expected 5 rows (1 header + 4 data), got {wb_check['MetaData'].max_row}"

    print("\n=== VALIDATION PASSED ===")
    print(f"Filename: {FILENAME}")
    print(f"TestPlan rows: 4")
    print(f"MetaData rows: 4")
    print(f"MetaData visibility: veryHidden")
    print(f"File size: {file_size} bytes")
    print("========================")

    wb_check.close()
    wb.close()

    return FILENAME, OUTPUT_PATH


if __name__ == "__main__":
    main()
