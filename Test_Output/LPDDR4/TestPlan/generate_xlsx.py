#!/usr/bin/env python3
"""LPDDR4 TestPlan XLSX Generator - Agent 7"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os, json, sys

IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime("%Y%m%d_%H%M%S")
filename = f"LPDDR4_TestPlan_{timestamp}.xlsx"

json_data = [
  {
    "Index": "1",
    "SS / Module": "LPDDR4",
    "Test Case Name": "lpddr4_inline_ecc_1bit_data_modify_test",
    "Feature": "Inline ECC 1-bit Corrected Error with Data Modify",
    "Test Description": "Verify LPDDR4 inline ECC 1-bit corrected error detection and interrupt generation across multiple ECC regions. The test configures ECC with a region map enabling regions 0, 2, 4, and 6, initializes memory via scrubber, writes random data to 7 regions, and reads back to verify that ECC-enabled regions produce corrected-error interrupts with correct data, while non-ECC regions produce data mismatches without interrupts. The final corrected error count is validated to equal 5.",
    "Speed": "SG3200 (default); SG2667 (if SG2667 defined); SG2133 (if SG2133 defined)",
    "Mode": "Full Bus (bus_width=0); DBI_EN=0; DM_EN=1; ECC_EN=1",
    "Memory Start Offset": "0x80000000 (region offset base, region*0x80000000 for scrub); port1_addr: 0x15A0000000 (APS_DRAM) / 0x11A0000000 (else)",
    "Memory End Offset": "address+0x1000 (scrub range end per region); region 6 end: 6*0x80000000 + 0x1000",
    "Remarks": "All three Agent 2 register-access macros (SYSREG_INTR_EN_ADDR, SYSREG_INTR_MSTS_ADDR, SYSREG_INTR_RSTCR_ADDR) are conditionally defined based on APS_DRAM vs MPS compile-time selection and were unresolved by Agent 3 and Agent 4.",
    "Test Steps / Procedure": "1. Enable the DDR controller interrupt via GIC and configure the system register interrupt enable for DDR controller interrupts.\n2. Perform GPV programming.\n3. Configure platform-specific base addresses for controller, PHY, and memory ports.\n4. Set bus width to full, disable DBI, enable DM and ECC, and configure speed grade.\n5. Execute LPDDR4 training sequence.\n6. Enable SBR done interrupt in the SII interrupt enable register.\n7. Disable software programming lock (SWCTL), program ECCCFG0 with ecc_region_map=0x55 and ECCCFG1 with 0x330, then re-enable the lock.\n8. For each ECC-enabled region (0, 2, 4, 6), perform memory initialization via scrubber: disable ports, configure scrub mode and interval, set scrub data pattern to zero, program scrub start/range addresses, enable scrub, wait for SBR done interrupt, reconfigure scrub parameters, and re-enable ports.\n9. Write random 64-bit data to all 7 regions (0-6) via port 1.\n10. Wait, signal training done, and wait again.\n11. Clear ECC control register, read region 0 data, poll ECCSTAT for corrected error bit, clear the corrected error, and clear the SII interrupt status.\n12. Validate that region 0 read data matches expected data.\n13. Enable both ECC corrected error and SBR done interrupts, and enable ECC corrected error interrupt in ECCCTL.\n14. For each of the 7 regions: read data, and if the region is ECC-enabled, validate data match and wait for corrected-error interrupt; if not ECC-enabled, validate data mismatch.\n15. Read ECCERRCNT and validate the corrected error count equals 5.\n16. Call finish with the error counter to determine pass or fail.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "1. After reading region 0, the ECC status register must indicate a corrected error (bit 8 set).\n2. Data read from ECC-enabled regions (0, 2, 4, 6) must match the originally written random data.\n3. Data read from non-ECC regions (1, 3, 5) must NOT match the originally written data.\n4. Each read from an ECC-enabled region must trigger a corrected-error interrupt, which is validated and cleared in the interrupt handler.\n5. The corrected error count register must equal 5 after all region reads.\n6. The system register masked interrupt status must correctly identify DDR controller interrupts.\n7. During scrubber initialization, the SBR done interrupt must fire and the scrub busy status must clear.\n8. The test passes if no errors are accumulated (error counter equals zero).",
    "Meta Test Description": "This testcase validates LPDDR4 inline ECC 1-bit corrected error detection with data modification across multiple ECC regions.",
    "Meta Test Steps / Procedure": "1. Declare local variables. 2. Set int_pend = 1. 3-38. Full execution flow.",
    "Meta Impacted Registers": "SYSREG_INTR_EN_ADDR; SYSREG_INTR_MSTS_ADDR; SYSREG_INTR_RSTCR_ADDR",
    "Meta Validation / Acceptance Criteria": "1. Poll ECCSTAT until bit 8 set. 2-11. Detailed validation.",
    "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mps_sysreg.h\"; \"aps_sysreg.h\"; \"lpddr4.h\"; \"lpddr4_aps_sii.h\"; \"lpddr4_sii.h\"",
    "Meta Macros": "#define CTL_INT_NO ... (conditional APS/MPS macros)",
    "Meta Arrays": "unsigned long int exp_data_array[50]; /* runtime populated */"
  },
  {
    "Index": "2",
    "SS / Module": "LPDDR4",
    "Test Case Name": "lpddr4_inline_ecc_2bit_data_modify_test",
    "Feature": "Inline ECC 2-bit Uncorrected Error with Data Modify",
    "Test Description": "Verify LPDDR4 inline ECC 2-bit uncorrected error detection, interrupt generation, and slave error behavior across multiple ECC regions.",
    "Speed": "SG3200 (default); SG2667 (if SG2667 defined); SG2133 (if SG2133 defined)",
    "Mode": "Full Bus (bus_width=0); DBI_EN=0; DM_EN=1; ECC_EN=1",
    "Memory Start Offset": "0x80000000 (region offset base); port1_addr: 0x15A0000000 (APS_DRAM) / 0x11A0000000 (else)",
    "Memory End Offset": "address+0x1000 (scrub range end per region); region 6 end: 6*0x80000000 + 0x1000",
    "Remarks": "All three Agent 2 register-access macros are conditionally defined and were unresolved. This test targets 2-bit uncorrected ECC errors with slave error tracking.",
    "Test Steps / Procedure": "1-16. Similar to 1-bit test but targeting uncorrected errors with slave error validation.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "1-9. Uncorrected error validation, slave errors, error count = 5.",
    "Meta Test Description": "This testcase validates LPDDR4 inline ECC 2-bit uncorrected error detection.",
    "Meta Test Steps / Procedure": "1-39. Full execution flow with slave error tracking.",
    "Meta Impacted Registers": "SYSREG_INTR_EN_ADDR; SYSREG_INTR_MSTS_ADDR; SYSREG_INTR_RSTCR_ADDR",
    "Meta Validation / Acceptance Criteria": "1-11. Detailed uncorrected error validation.",
    "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mps_sysreg.h\"; \"aps_sysreg.h\"; \"lpddr4.h\"; \"lpddr4_aps_sii.h\"; \"lpddr4_sii.h\"",
    "Meta Macros": "#define CTL_INT_NO ... (conditional APS/MPS macros)",
    "Meta Arrays": "unsigned long int exp_data_array[50]; /* runtime populated */"
  },
  {
    "Index": "3",
    "SS / Module": "LPDDR4",
    "Test Case Name": "lpddr4_mem_dm_test",
    "Feature": "Memory Data Masking (DM)",
    "Test Description": "Verify LPDDR4 memory data masking (DM) functionality by writing random data patterns and reading them back.",
    "Speed": "SG3200 (default); SG2667 (if SG2667 defined); SG2133 (if SG2133 defined)",
    "Mode": "Full Bus (bus_width=0); DBI_EN=0; DM_EN=1",
    "Memory Start Offset": "port0_addr: 0; port1_addr: 0x15A0000000 (APS_DRAM) / 0x11A0000000 (else)",
    "Memory End Offset": "port0_addr + 0x9000; port1_addr + 0x3E0000000",
    "Remarks": "Pure memory read/write test with DM enabled. No DDR controller register accesses.",
    "Test Steps / Procedure": "1-10. GPV programming, training, write/read/validate phases.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "1-4. Data integrity and cross-port coherency validation.",
    "Meta Test Description": "This testcase validates LPDDR4 memory data masking (DM) functionality.",
    "Meta Test Steps / Procedure": "1-18. Full execution flow.",
    "Meta Impacted Registers": "NA",
    "Meta Validation / Acceptance Criteria": "1-4. Port0/port1 read-back validation.",
    "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"lpddr4.h\"",
    "Meta Macros": "NA",
    "Meta Arrays": "unsigned long int exp_data_array[50]; /* runtime populated */"
  },
  {
    "Index": "4",
    "SS / Module": "LPDDR4",
    "Test Case Name": "lpddr4_mem_half_data_bus_width_test",
    "Feature": "Half Data Bus Width Memory Access",
    "Test Description": "Verify LPDDR4 memory read access in half data bus width mode.",
    "Speed": "SG3200 (default); SG2667 (if SG2667 defined); SG2133 (if SG2133 defined)",
    "Mode": "Half Bus (bus_width=1); DBI_EN=0; DM_EN=0",
    "Memory Start Offset": "port0_addr: 0; port1_addr: 0x15A0000000 (APS_DRAM) / 0x11A0000000 (else)",
    "Memory End Offset": "port0_addr + 0x108; port1_addr + 0x108",
    "Remarks": "Read-only memory verification test in half data bus width mode. No data written. DM disabled.",
    "Test Steps / Procedure": "1-8. GPV programming, training, read/validate.",
    "Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "1-3. Fixed pattern validation with AND logic.",
    "Meta Test Description": "This testcase validates LPDDR4 memory access in half data bus width mode.",
    "Meta Test Steps / Procedure": "1-19. Full execution flow.",
    "Meta Impacted Registers": "NA",
    "Meta Validation / Acceptance Criteria": "1-3. Port0/port1 read validation.",
    "Meta Headers": "<stdio.h>; <stdlib.h>; \"test_common.h\"; \"lpddr4.h\"",
    "Meta Macros": "NA",
    "Meta Arrays": "NA"
  }
]

print(f"Generating: {filename}")
print(f"Timestamp (IST): {now_ist.strftime('%Y-%m-%d %H:%M:%S %Z')}")
