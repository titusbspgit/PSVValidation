#!/usr/bin/env python3
"""LPDDR4 TestPlan Excel Generator - Agent 7
Generates LPDDR4_TestPlan_<YYYYMMDD>_<HHMMSS>.xlsx using openpyxl.
This script is executed by GitHub Actions workflow.
"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os
import json
import sys

def main():
    IST = timezone(timedelta(hours=5, minutes=30))
    now_ist = datetime.now(IST)
    timestamp = now_ist.strftime("%Y%m%d_%H%M%S")

    json_data = [
        {
            "Index": "1",
            "SS / Module": "LPDDR4",
            "Test Case Name": "lpddr4_inline_ecc_1bit_data_modify_test",
            "Feature": "Inline ECC",
            "Meta Headers": '<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mps_sysreg.h\"; \"aps_sysreg.h\"; \"lpddr4.h\"; \"lpddr4_aps_sii.h\"; \"lpddr4_sii.h\"',
            "Meta Macros": "CTL_INT_NO; PHY_INT_NO; SYSREG_INTR_EN_ADDR; SYSREG_CTRL_INTR_EN_MASK; SYSREG_PHY_INTR_EN_MASK; SYSREG_INTR_MSTS_ADDR; SYSREG_CTRL_INTR_MSTS_MASK; SYSREG_PHY_INTR_MSTS_MASK; SYSREG_INTR_RSTCR_ADDR; SYSREG_CTRL_INTR_RSTCR_MASK; SYSREG_PHY_INTR_RSTCR_MASK; SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK; SII_INTR_EN_SBR_DONE_INTR_MASK",
            "Meta Arrays": "exp_data_array[50]",
            "Speed": "NA",
            "Mode": "Interrupt Mode",
            "Memory Start Offset": "NA",
            "Memory End Offset": "NA",
            "Meta Test Description": "This testcase validates LPDDR4 inline ECC 1-bit corrected error detection with data modification across multiple ECC regions. It configures ECC with ecc_region_map=0x55 (regions 0, 2, 4, 6 enabled), initializes memory via scrubber (SBR) for each enabled region, writes random 64-bit data to port1 addresses across 7 regions, performs LPDDR4 training, then reads back data and checks for ECC corrected errors. For ECC-enabled regions, it verifies data matches and that a corrected-error interrupt fires (ECCSTAT bit[8]). For non-ECC regions, it verifies data mismatch (no ECC protection). It enables ECC corrected error interrupt via ECCCTL bit[8] and SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK, then iterates all 7 regions validating interrupt behavior and data integrity. Finally, it reads ECCERRCNT and verifies the corrected error count equals 5. The IRQ handler (Default_IRQHandler) reads SYSREG_INTR_MSTS_ADDR, checks for SBR done or ECC corrected error interrupts, clears them via ECCCTL, SII register, and SYSREG_INTR_RSTCR_ADDR, and calls GIC_ClearIRQ. The memory_init_scrb function disables ports PCTRL_0 and PCTRL_1, configures SBRCTL for scrub_mode=1 with scrub_interval=0, programs SBRWDATA0/SBRWDATA1 with zeros, sets SBRSTART0/SBRSTART1 and SBRRANGE0/SBRRANGE1 for the target address range, enables scrubbing, waits for SBR done interrupt, then restores scrub_mode=0 and re-enables ports.",
            "Test Description": "Validates LPDDR4 inline ECC 1-bit corrected error detection and interrupt handling across multiple memory regions. The test configures ECC with a region map enabling alternating regions, initializes memory using the scrubber, writes random data to each region, then reads back data to trigger ECC corrected error detection. For ECC-enabled regions, it verifies data integrity and that a corrected-error interrupt is generated and properly handled. For non-ECC regions, it confirms data mismatch due to lack of ECC protection. Finally, it verifies the total corrected error count matches the expected value of 5.",
            "Meta Test Steps / Procedure": "1. Set int_pend=1, enable GIC IRQ for CTL_INT_NO via GIC_EnableIRQ(CTL_INT_NO).\n2. Write SYSREG_INTR_EN_ADDR with SYSREG_CTRL_INTR_EN_MASK to enable controller interrupt.\n3. Call gpv_programming() for bus configuration.\n4. Set err0=0.\n5. Configure port0_addr, port1_addr, ctl_base, phy_base based on APS_DRAM or MPS_DRAM conditional compilation.\n6. Set bus_width=0 (Full Bus), DBI_EN=0, DM_EN=1, ECC_EN=1.\n7. Set SG speed grade based on SG2667/SG2133/default(3200) defines.\n8. Call lpddr4_training().\n9. Write ctl_base+0x0000819C with SII_INTR_EN_SBR_DONE_INTR_MASK to enable SBR done interrupt.\n10. Set ecc_region_map=0x55.\n11. Write ctl_base+0x00000320 (SWCTL) with 0x00000000 to disable quasi-dynamic programming.\n12. Write ctl_base+0x00000070 (ECCCFG0) with (ecc_region_map<<8)|0xb4.\n13. Write ctl_base+0x00000074 (ECCCFG1) with 0x00000330.\n14. Write ctl_base+0x00000320 (SWCTL) with 0x00000001 to re-enable.\n15. Set int_expected=0.\n16. Set offset=0x80000000.\n17. Loop region 0..6: if ecc_region_map bit for region is set, call memory_init_scrb(offsetregion).\n18. Loop region 0..6: generate random 64-bit data in exp_data_array[region], write via write_reg64 to port1_addr+(regionoffset).\n19. Call wait_on(1000), training_done(), wait_on(1000).\n20. Write ctl_base+0x0000007c (ECCCTL) with 0x00000000.\n21. Read port1_addr via read_reg64 into read_data.\n22. Read ctl_base+0x00000078 (ECCSTAT) into eccstat.\n23. Poll ECCSTAT bit[8] until set (while !(eccstat&(1<<8))).\n24. Read-modify-write ctl_base+0x0000007c (ECCCTL) OR with 0x00000001 to clear corrected error.\n25. Write ctl_base+0x00008194 with SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK.\n26. Call wait_on(1000).\n27. Compare exp_data_array[0] with read_data; if mismatch, print error, increment err0.\n28. Write ctl_base+0x0000819C with SII_ECC_CORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK.\n29. Write ctl_base+0x0000007c (ECCCTL) with 1<<8 to enable ECC corrected error interrupt.\n30. Loop region 0..6: set int_expected=(ecc_region_map>>region)&0x1, set int_pend1=1, read port1_addr+(regionoffset) via read_reg64, call wait_on(1000).\n31. If int_expected: compare exp_data_array[region] with read_data (error if mismatch), poll int_pend1 until 0 (wait_on(10) loop).\n32. If not int_expected: compare exp_data_array[region] with read_data (error if match, meaning data should differ).\n33. Read ctl_base+0x00000080 (ECCERRCNT) into count.\n34. If count != 5, print error, increment err0.\n35. Call finish(err0).\n36. IRQ Handler: read SYSREG_INTR_MSTS_ADDR, check against SYSREG_CTRL_INTR_MSTS_MASK. If sbr_init: read ctl_base+0x00008194, check SII_INTR_EN_SBR_DONE_INTR_MASK, poll ctl_base+0x00000f28 (SBRRANGE1) bit[0] until 0, write SBRCTL to disable scrub, write SII register to clear SBR interrupt, set int_pend1=0. If not sbr_init: read ECCSTAT, check bit[8], read-modify-write ECCCTL OR 0x00000001, write SII register to clear ECC interrupt, set int_pend1=0. Write SYSREG_INTR_RSTCR_ADDR with SYSREG_CTRL_INTR_RSTCR_MASK, call GIC_ClearIRQ(CTL_INT_NO).\n37. memory_init_scrb: disable PCTRL_0 (ctl_base+0x00000490=0) and PCTRL_1 (ctl_base+0x00000540=0). Set SBRCTL scrub_mode=1 (bit[2]), scrub_interval=0 (bits[31:8]). Write SBRWDATA0 (ctl_base+0x00000f2c=0) and SBRWDATA1 (ctl_base+0x00000f30=0). Set SBRSTART0/SBRSTART1 and SBRRANGE0/SBRRANGE1 for target address. Enable scrub (SBRCTL bit[0]=1). Wait for SBR done interrupt. Restore scrub_mode=0, scrub_interval=1, disable scrub. Re-enable PCTRL_0 and PCTRL_1.",
            "Test Steps / Procedure": "1. Enable the GIC IRQ for the LPDDR4 controller interrupt and enable the controller interrupt in the system register interrupt enable register.\n2. Perform bus (GPV) programming.\n3. Configure port addresses, controller base, and PHY base for the target subsystem.\n4. Set bus width to full, enable data mask and ECC, disable DBI.\n5. Perform LPDDR4 training.\n6. Enable the SBR done interrupt in the SII interrupt enable register.\n7. Set ECC region map to enable alternating regions (regions 0, 2, 4, 6).\n8. Disable quasi-dynamic register programming via SWCTL, configure ECCCFG0 with the ECC region map and ECCCFG1, then re-enable SWCTL.\n9. For each ECC-enabled region, initialize memory using the scrubber (disable ports, configure scrub mode, write zero pattern, set scrub address range, enable scrub, wait for SBR done interrupt, restore ports).\n10. Write random 64-bit data to each of the 7 regions via the secondary port.\n11. Wait, complete training, and wait again.\n12. Clear ECCCTL, read data from region 0 via the secondary port, and poll ECCSTAT for corrected error status (bit 8).\n13. Clear the corrected error via ECCCTL and the SII interrupt status register.\n14. Verify region 0 read data matches the expected written data.\n15. Enable ECC corrected error interrupt in both the SII interrupt enable register and ECCCTL.\n16. For each of the 7 regions, read data via the secondary port and verify: for ECC-enabled regions, confirm data matches and the corrected-error interrupt fires; for non-ECC regions, confirm data does not match.\n17. Read ECCERRCNT and verify the corrected error count equals 5.\n18. Report test pass or fail based on accumulated error count.",
            "Meta Impacted Registers": "SYSREG_INTR_EN_ADDR; SYSREG_INTR_MSTS_ADDR; SYSREG_INTR_RSTCR_ADDR",
            "Impacted Registers": "NA",
            "Meta Validation / Acceptance Criteria": "1. After initial read of region 0 with ECC interrupt disabled: exp_data_array[0] must equal read_data, otherwise error incremented.\n2. ECCSTAT (ctl_base+0x00000078) bit[8] must be set after read from ECC-enabled region, indicating corrected error detected.\n3. For each ECC-enabled region (ecc_region_map bit set): exp_data_array[region] must equal read_data (data integrity preserved by ECC correction), and int_pend1 must become 0 (interrupt must fire and be handled).\n4. For each non-ECC region (ecc_region_map bit clear): exp_data_array[region] must NOT equal read_data (data should differ since no ECC protection).\n5. ECCERRCNT (ctl_base+0x00000080) must equal 5 after all region reads.\n6. In IRQ handler: SYSREG_INTR_MSTS_ADDR must read SYSREG_CTRL_INTR_MSTS_MASK. For ECC interrupt: ECCSTAT bit[8] must be 0x100. For SBR interrupt: SII register must contain SII_INTR_EN_SBR_DONE_INTR_MASK, and SBRRANGE1 bit[0] must become 0.\n7. err0 must be 0 for test pass via finish(err0).",
            "Validation / Acceptance Criteria": "1. Data read from ECC-enabled regions must match the originally written random data, confirming ECC correction preserved data integrity.\n2. ECCSTAT register must indicate a corrected error (bit 8 set) after reading from an ECC-enabled region.\n3. A corrected-error interrupt must fire for each ECC-enabled region and be properly handled (cleared via ECCCTL and SII interrupt status).\n4. Data read from non-ECC regions must NOT match the originally written data, confirming absence of ECC protection.\n5. The corrected error count register (ECCERRCNT) must equal 5 after reading all regions.\n6. The interrupt handler must correctly identify the interrupt source via the system register masked status, clear the interrupt via the system register reset/clear register, and clear the GIC IRQ.\n7. The test must complete with zero errors for a pass verdict.",
            "Remarks": "Test uses conditional compilation (APS_DRAM vs MPS_DRAM) for subsystem-specific addresses and interrupt numbers. ECC region map 0x55 enables regions 0, 2, 4, 6 (4 out of 7 regions). The expected corrected error count of 5 includes the initial region 0 read plus 4 ECC-enabled region reads in the loop. Memory initialization uses the scrubber (SBR) with interrupt-driven completion. All three Agent 4 register mappings are unresolved."
        },
        {
            "Index": "2",
            "SS / Module": "LPDDR4",
            "Test Case Name": "lpddr4_inline_ecc_2bit_data_modify_test",
            "Feature": "Inline ECC",
            "Meta Headers": '<stdio.h>; <stdlib.h>; \"test_common.h\"; \"mps_sysreg.h\"; \"aps_sysreg.h\"; \"lpddr4.h\"; \"lpddr4_aps_sii.h\"; \"lpddr4_sii.h\"',
            "Meta Macros": "CTL_INT_NO; PHY_INT_NO; SYSREG_INTR_EN_ADDR; SYSREG_CTRL_INTR_EN_MASK; SYSREG_PHY_INTR_EN_MASK; SYSREG_INTR_MSTS_ADDR; SYSREG_CTRL_INTR_MSTS_MASK; SYSREG_PHY_INTR_MSTS_MASK; SYSREG_INTR_RSTCR_ADDR; SYSREG_CTRL_INTR_RSTCR_MASK; SYSREG_PHY_INTR_RSTCR_MASK; SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK; SII_INTR_EN_SBR_DONE_INTR_MASK",
            "Meta Arrays": "exp_data_array[50]",
            "Speed": "NA",
            "Mode": "Interrupt Mode",
            "Memory Start Offset": "NA",
            "Memory End Offset": "NA",
            "Meta Test Description": "This testcase validates LPDDR4 inline ECC 2-bit uncorrectable error detection with data modification across multiple ECC regions. It configures ECC with ecc_region_map=0x55 (regions 0, 2, 4, 6 enabled), initializes memory via scrubber (SBR) for each enabled region, writes random 64-bit data to port1 addresses across 7 regions, performs LPDDR4 training, then reads back data and checks for ECC uncorrectable errors. It clears ECCCTL, reads region 0 from port1, and polls ECCSTAT bit[16] for uncorrectable error detection. It clears the uncorrectable error via ECCCTL bit[1] (OR with 0x00000002) and writes SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK to the SII status register (ctl_base+0x00008194). It then enables ECC uncorrectable error interrupt via ECCCTL bit[9] and SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK in the SII interrupt enable register (ctl_base+0x0000819C). For each of 7 regions, it reads data from port1, and for ECC-enabled regions it waits for the interrupt (int_pend1 becomes 0) and checks that slverr_taken==1 (slave error expected on uncorrectable ECC error). For all regions, it verifies that exp_data_array[region] does NOT equal read_data (data should never match since 2-bit errors are uncorrectable). Finally, it reads ECCERRCNT (ctl_base+0x00000080) and verifies the uncorrectable error count (bits[31:16]) equals 5, and num_slverr_taken equals 5. The IRQ handler (Default_IRQHandler) reads SYSREG_INTR_MSTS_ADDR, checks for SBR done or ECC uncorrectable error interrupts, clears them via ECCCTL (OR 0x00000002), SII register, and SYSREG_INTR_RSTCR_ADDR, and calls GIC_ClearIRQ. Slave error handlers (lower_el_aarch64_irq_vector, curr_el_spx_fiq_vector) set slverr_taken=1 and increment num_slverr_taken. The memory_init_scrb function disables ports PCTRL_0 and PCTRL_1, configures SBRCTL for scrub_mode=1 with scrub_interval=0, programs SBRWDATA0/SBRWDATA1 with zeros, sets SBRSTART0/SBRSTART1 and SBRRANGE0/SBRRANGE1 for the target address range, enables scrubbing, waits for SBR done interrupt, then restores scrub_mode=0 and re-enables ports.",
            "Test Description": "Validates LPDDR4 inline ECC 2-bit uncorrectable error detection, interrupt handling, and slave error behavior across multiple memory regions. The test configures ECC with a region map enabling alternating regions (0, 2, 4, 6), initializes memory using the scrubber, writes random 64-bit data to each of 7 regions, then reads back data to trigger ECC uncorrectable error detection. For ECC-enabled regions, it verifies that an uncorrectable-error interrupt fires and a slave error is taken. For all regions, it confirms data does not match the originally written data since 2-bit errors are uncorrectable. Finally, it verifies the total uncorrectable error count equals 5 and the total slave error count equals 5.",
            "Meta Test Steps / Procedure": "1. Set int_pend=1, num_slverr_taken=0, enable GIC IRQ for CTL_INT_NO via GIC_EnableIRQ(CTL_INT_NO).\n2. Write SYSREG_INTR_EN_ADDR with SYSREG_CTRL_INTR_EN_MASK to enable controller interrupt.\n3. Call gpv_programming() for bus configuration.\n4. Set err0=0.\n5. Configure port0_addr, port1_addr, ctl_base, phy_base based on APS_DRAM or MPS_DRAM conditional compilation.\n6. Set bus_width=0 (Full Bus), DBI_EN=0, DM_EN=1, ECC_EN=1.\n7. Set SG speed grade based on SG2667/SG2133/default(3200) defines.\n8. Call lpddr4_training().\n9. Write ctl_base+0x0000819C with SII_INTR_EN_SBR_DONE_INTR_MASK to enable SBR done interrupt.\n10. Set ecc_region_map=0x55.\n11. Write ctl_base+0x00000320 (SWCTL) with 0x00000000 to disable quasi-dynamic programming.\n12. Write ctl_base+0x00000070 (ECCCFG0) with (ecc_region_map<<8)|0xb4.\n13. Write ctl_base+0x00000074 (ECCCFG1) with 0x00000330.\n14. Write ctl_base+0x00000320 (SWCTL) with 0x00000001 to re-enable.\n15. Set int_expected=0.\n16. Set offset=0x80000000.\n17. Loop region 0..6: if ecc_region_map bit for region is set, call memory_init_scrb(offsetregion).\n18. Loop region 0..6: generate random 64-bit data in exp_data_array[region], write via write_reg64 to port1_addr+(regionoffset).\n19. Call wait_on(1000), training_done(), wait_on(1000).\n20. Write ctl_base+0x0000007c (ECCCTL) with 0x00000000.\n21. Read port1_addr via read_reg64 into read_data.\n22. Read ctl_base+0x00000078 (ECCSTAT) into eccstat.\n23. Poll ECCSTAT bit[16] until set (while !(eccstat&(1<<16))).\n24. Read-modify-write ctl_base+0x0000007c (ECCCTL) OR with 0x00000002 to clear uncorrectable error.\n25. Write ctl_base+0x00008194 with SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK.\n26. Call wait_on(1000).\n27. Write ctl_base+0x0000819C with SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK | SII_INTR_EN_SBR_DONE_INTR_MASK to enable interrupts.\n28. Write ctl_base+0x0000007c (ECCCTL) with 1<<9 to enable ECC uncorrectable error interrupt.\n29. Loop region 0..6: set slverr_taken=0, int_expected=(ecc_region_map>>region)&0x1, int_pend1=1, read port1_addr+(regionoffset) via read_reg64, call wait_on(1000).\n30. If int_expected: poll int_pend1 until 0 (wait_on(10) loop), check slverr_taken==1 (error if 0, increment err0).\n31. For all regions: compare exp_data_array[region] with read_data (error if match, since 2-bit errors are uncorrectable, data should never match), increment err0 on match.\n32. Read ctl_base+0x00000080 (ECCERRCNT) into count.\n33. If (count>>16) != 5, print error, increment err0.\n34. If num_slverr_taken != 5, print error, increment err0.\n35. Call finish(err0).\n36. IRQ Handler (Default_IRQHandler): read SYSREG_INTR_MSTS_ADDR, check against SYSREG_CTRL_INTR_MSTS_MASK. If sbr_init: read ctl_base+0x00008194, check SII_INTR_EN_SBR_DONE_INTR_MASK, poll ctl_base+0x00000f28 (SBRRANGE1) bit[0] until 0, write SBRCTL to disable scrub via set_data bit[0]=0, write SII register to clear SBR interrupt, set int_pend1=0. If not sbr_init: check int_expected (error if 0), read ECCSTAT, check bit[16]==0x10000, read-modify-write ECCCTL OR 0x00000002, write SII register with SII_ECC_UNCORRECTED_ERR_INTR_EN_FAULT_MASK to clear ECC interrupt, set int_pend1=0. Write SYSREG_INTR_RSTCR_ADDR with SYSREG_CTRL_INTR_RSTCR_MASK, call GIC_ClearIRQ(CTL_INT_NO).\n37. Slave error handlers (lower_el_aarch64_irq_vector, curr_el_spx_fiq_vector): set slverr_taken=1, increment num_slverr_taken, execute ISB SY and DAIFClr #0xF instructions.\n38. memory_init_scrb: set sbr_init=1. Disable PCTRL_0 (ctl_base+0x00000490=0) and PCTRL_1 (ctl_base+0x00000540=0). Read-modify-write SBRCTL (ctl_base+0x00000f24) to set scrub_mode=1 (bit[2]). Read-modify-write SBRCTL to set scrub_interval=0 (bits[31:8]). Write SBRWDATA0 (ctl_base+0x00000f2c=0) and SBRWDATA1 (ctl_base+0x00000f30=0). Set int_pend1=1. Write SBRSTART0 (ctl_base+0x00000f38) and SBRSTART1 (ctl_base+0x00000f3c) with (address>>3). Write SBRRANGE0 (ctl_base+0x00000f40) and SBRRANGE1 (ctl_base+0x00000f44) with ((address+0x1000)>>3). Enable scrub via SBRCTL bit[0]=1. Wait for SBR done interrupt (poll int_pend1). Read SBRCTL, set scrub_mode=0 (bit[2]), scrub_interval=1 (bits[31:8]), write back. Disable scrub via SBRCTL bit[0]=0. Re-enable PCTRL_0 (ctl_base+0x00000490=1) and PCTRL_1 (ctl_base+0x00000540=1). Set sbr_init=0.",
            "Test Steps / Procedure": "1. Enable the GIC IRQ for the LPDDR4 controller interrupt and enable the controller interrupt in the system register interrupt enable register.\n2. Perform bus (GPV) programming.\n3. Configure port addresses, controller base, and PHY base for the target subsystem.\n4. Set bus width to full, enable data mask and ECC, disable DBI.\n5. Perform LPDDR4 training.\n6. Enable the SBR done interrupt in the SII interrupt enable register.\n7. Set ECC region map to enable alternating regions (regions 0, 2, 4, 6).\n8. Disable quasi-dynamic register programming via SWCTL, configure ECCCFG0 with the ECC region map and ECCCFG1, then re-enable SWCTL.\n9. For each ECC-enabled region, initialize memory using the scrubber (disable ports, configure scrub mode with zero pattern, set scrub address range, enable scrub, wait for SBR done interrupt, restore scrub settings, re-enable ports).\n10. Write random 64-bit data to each of the 7 regions via the secondary port.\n11. Wait, complete training, and wait again.\n12. Clear ECCCTL, read data from region 0 via the secondary port, and poll ECCSTAT for uncorrectable error status (bit 16).\n13. Clear the uncorrectable error via ECCCTL and the SII interrupt status register.\n14. Enable ECC uncorrectable error interrupt in both the SII interrupt enable register and ECCCTL.\n15. For each of the 7 regions, read data via the secondary port and verify: for ECC-enabled regions, confirm the uncorrectable-error interrupt fires and a slave error is taken; for all regions, confirm data does not match the originally written data.\n16. Read ECCERRCNT and verify the uncorrectable error count (upper 16 bits) equals 5.\n17. Verify the total slave error count equals 5.\n18. Report test pass or fail based on accumulated error count.",
            "Meta Impacted Registers": "SYSREG_INTR_EN_ADDR; SYSREG_INTR_MSTS_ADDR; SYSREG_INTR_RSTCR_ADDR",
            "Impacted Registers": "NA",
            "Meta Validation / Acceptance Criteria": "1. After initial read of region 0 with ECC interrupt disabled: ECCSTAT (ctl_base+0x00000078) bit[16] must be set (0x10000), indicating uncorrectable error detected.\n2. For each ECC-enabled region (ecc_region_map bit set): int_pend1 must become 0 (interrupt must fire and be handled), and slverr_taken must equal 1 (slave error expected on uncorrectable ECC error).\n3. For all 7 regions: exp_data_array[region] must NOT equal read_data (data should never match since 2-bit errors are uncorrectable; error if data matches).\n4. ECCERRCNT (ctl_base+0x00000080) bits[31:16] ((count>>16)) must equal 5 after all region reads.\n5. num_slverr_taken must equal 5 after all region reads.\n6. In IRQ handler: SYSREG_INTR_MSTS_ADDR must read SYSREG_CTRL_INTR_MSTS_MASK. For ECC uncorrectable interrupt: ECCSTAT bit[16] must be 0x10000, ECCCTL is OR'd with 0x00000002 to clear. For SBR interrupt: SII register must contain SII_INTR_EN_SBR_DONE_INTR_MASK, and SBRRANGE1 bit[0] must become 0.\n7. In slave error handlers: slverr_taken must be set to 1, num_slverr_taken must be incremented.\n8. err0 must be 0 for test pass via finish(err0).",
            "Validation / Acceptance Criteria": "1. ECCSTAT register must indicate an uncorrectable error (bit 16 set) after reading from an ECC-enabled region.\n2. An uncorrectable-error interrupt must fire for each ECC-enabled region and be properly handled (cleared via ECCCTL and SII interrupt status).\n3. A slave error must be taken for each ECC-enabled region read, confirmed by the slave error handler being invoked.\n4. Data read from all regions must NOT match the originally written data, confirming that 2-bit errors are uncorrectable and data integrity is not preserved.\n5. The uncorrectable error count in ECCERRCNT (upper 16 bits) must equal 5 after reading all regions.\n6. The total slave error count must equal 5.\n7. The interrupt handler must correctly identify the interrupt source via the system register masked status, clear the interrupt via the system register reset/clear register, and clear the GIC IRQ.\n8. The test must complete with zero errors for a pass verdict.",
            "Remarks": "Test uses conditional compilation (APS_DRAM vs MPS_DRAM) for subsystem-specific addresses and interrupt numbers. ECC region map 0x55 enables regions 0, 2, 4, 6 (4 out of 7 regions). This test validates 2-bit uncorrectable errors where data cannot be corrected, so data mismatch is expected for all regions including ECC-enabled ones. The test additionally validates slave error behavior via lower_el_aarch64_irq_vector and curr_el_spx_fiq_vector handlers with ISB SY and DAIFClr instructions. The expected uncorrectable error count of 5 includes the initial region 0 read plus 4 ECC-enabled region reads in the loop. All three Agent 4 register mappings are unresolved."
        },
        {
            "Index": "3",
            "SS / Module": "LPDDR4",
            "Test Case Name": "lpddr4_mem_dm_test",
            "Feature": "Data Mask",
            "Meta Headers": '<stdio.h>; <stdlib.h>; \"test_common.h\"; \"lpddr4.h\"',
            "Meta Macros": "NA",
            "Meta Arrays": "exp_data_array[50]",
            "Speed": "NA",
            "Mode": "NA",
            "Memory Start Offset": "0x1000; 0x20000000",
            "Memory End Offset": "NA",
            "Meta Test Description": "This testcase validates LPDDR4 memory data mask (DM) functionality by performing write and read-back verification across two memory access patterns. It configures DM_EN=1, DBI_EN=0, bus_width=0 (Full Bus), and performs lpddr4_training(). The test has two phases. Phase 1 (conditional, guarded by initiator/subsystem preprocessor checks): writes 10 random 64-bit values to port0 at offset=0x1000 intervals, reads them back from port0 and verifies, then reads the same addresses from port1 and verifies. Phase 2 (unconditional): writes 32 random 64-bit values to port1 at offset=0x20000000 intervals, reads them back from port1 and verifies. Random data is generated using rand() for lower 32 bits and rand()<<32 for upper 32 bits, combined via OR. Each mismatch increments err0. The test calls finish(err0) to report pass/fail. Port addresses and controller/PHY bases are set via APS_DRAM/MPS_DRAM conditional compilation. Speed grade is set via SG2667/SG2133/default(3200) defines.",
            "Test Description": "Validates LPDDR4 memory data mask functionality by writing random 64-bit data patterns and reading them back to verify data integrity. The test enables data mask, performs LPDDR4 training, then executes two phases: first, conditionally writes and reads 10 entries at a smaller offset via both port 0 and port 1 to verify cross-port data consistency; second, unconditionally writes and reads 32 entries at a larger offset via port 1 to verify data integrity across a wider address range. Any data mismatch is flagged as an error.",
            "Meta Test Steps / Procedure": "1. Call gpv_programming() for bus configuration.\n2. Set err0=0.\n3. Configure port0_addr, port1_addr, ctl_base, phy_base based on APS_DRAM or MPS_DRAM conditional compilation. APS_DRAM: port0_addr=0, port1_addr=0x15A0000000, ctl_base=0x9EE03000, phy_base=0x9F000000. MPS_DRAM: port0_addr=0, port1_addr=0x11A0000000, ctl_base=0x11D003000, phy_base=0x11D500000.\n4. Set bus_width=0 (Full Bus).\n5. Set SG speed grade based on SG2667/SG2133/default(3200) defines.\n6. Set DBI_EN=0, DM_EN=1.\n7. Call lpddr4_training().\n8. Phase 1 (conditional on initiator/subsystem preprocessor guard: !((APS_DRAM && A53_INITIATOR) ^ (MPS_DRAM && AI_INITIATOR) ^ (MPS_DRAM && DSP_INITIATOR))):\n a. Set offset=0x1000.\n b. Loop index 0..9: generate random 64-bit data via rand() | ((unsigned long)rand()<<32), store in exp_data_array[index], write via write_reg64 to port0_addr+(indexoffset).\n c. Loop index 0..9: read via read_reg64 from port0_addr+(indexoffset) into read_data, compare with exp_data_array[index], print error and increment err0 on mismatch.\n d. Loop index 0..9: read via read_reg64 from port1_addr+(indexoffset) into read_data, compare with exp_data_array[index], print error and increment err0 on mismatch.\n9. Phase 2 (unconditional):\n a. Set offset=0x20000000.\n b. Loop index 0..31: generate random 64-bit data via rand() | ((unsigned long)rand()<<32), store in exp_data_array[index], write via write_reg64 to port1_addr+(indexoffset).\n c. Loop index 0..31: read via read_reg64 from port1_addr+(indexoffset) into read_data, compare with exp_data_array[index], print error and increment err0 on mismatch.\n10. Call finish(err0).",
            "Test Steps / Procedure": "1. Perform bus (GPV) programming.\n2. Configure port addresses, controller base, and PHY base for the target subsystem.\n3. Set bus width to full, enable data mask, disable DBI.\n4. Perform LPDDR4 training.\n5. (Conditional phase) Write 10 random 64-bit data values to sequential addresses via port 0 with a small address stride.\n6. (Conditional phase) Read back the 10 values from port 0 and verify each matches the expected written data.\n7. (Conditional phase) Read back the same 10 values from port 1 and verify each matches the expected written data.\n8. Write 32 random 64-bit data values to sequential addresses via port 1 with a large address stride.\n9. Read back the 32 values from port 1 and verify each matches the expected written data.\n10. Report test pass or fail based on accumulated error count.",
            "Meta Impacted Registers": "NA",
            "Impacted Registers": "NA",
            "Meta Validation / Acceptance Criteria": "1. Phase 1 port0 read-back: For each index 0..9, read_data from port0_addr+(index0x1000) must equal exp_data_array[index]. Mismatch prints ERROR_0 with port0 label and increments err0.\n2. Phase 1 port1 read-back: For each index 0..9, read_data from port1_addr+(index0x1000) must equal exp_data_array[index]. Mismatch prints ERROR_0 with port1 label and increments err0.\n3. Phase 2 port1 read-back: For each index 0..31, read_data from port1_addr+(index0x20000000) must equal exp_data_array[index]. Mismatch prints ERROR_1 with port1 label and increments err0.\n4. err0 must be 0 for test pass via finish(err0).",
            "Validation / Acceptance Criteria": "1. All 10 data values written to port 0 must be read back correctly from port 0, confirming data integrity with data mask enabled.\n2. The same 10 data values must be read back correctly from port 1, confirming cross-port data consistency.\n3. All 32 data values written to port 1 at a large address stride must be read back correctly from port 1, confirming data integrity across a wide address range.\n4. The test must complete with zero errors for a pass verdict.",
            "Remarks": "Test uses conditional compilation (APS_DRAM vs MPS_DRAM) for subsystem-specific addresses. Phase 1 is additionally guarded by an initiator/subsystem preprocessor condition involving A53_INITIATOR, AI_INITIATOR, and DSP_INITIATOR defines. DM_EN is set to 1 to enable data mask; DBI is disabled. No register-access macros or mapped registers were identified by Agent 2 or Agent 4. The test relies on write_reg64 and read_reg64 helper functions for 64-bit memory access."
        },
        {
            "Index": "4",
            "SS / Module": "LPDDR4",
            "Test Case Name": "lpddr4_mem_half_data_bus_width_test",
            "Feature": "Half Data Bus Width",
            "Meta Headers": '<stdio.h>; <stdlib.h>; \"test_common.h\"; \"lpddr4.h\"',
            "Meta Macros": "NA",
            "Meta Arrays": "NA",
            "Speed": "NA",
            "Mode": "NA",
            "Memory Start Offset": "NA",
            "Memory End Offset": "NA",
            "Meta Test Description": "This testcase validates LPDDR4 memory access in half data bus width mode. It configures bus_width=1 (Half Bus), DBI_EN=0, DM_EN=0, and performs lpddr4_training() followed by training_done() and wait_on(1000). The test then reads two 64-bit values from fixed addresses. Phase 1 (conditional, guarded by initiator/subsystem preprocessor check): reads port0_addr+0x100 into read_data0 and port0_addr+0x108 into read_data1 via read_reg64, and checks that read_data0 equals 0x3333333333333333 AND read_data1 equals 0x2222222222222222 (error if both conditions fail simultaneously due to && logic). Phase 2 (unconditional): reads port1_addr+0x100 into read_data2 and port1_addr+0x108 into read_data3 via read_reg64, and checks that read_data2 equals 0x3333333333333333 AND read_data3 equals 0x2222222222222222 (error if both conditions fail). Port addresses and controller/PHY bases are set via APS_DRAM/MPS_DRAM conditional compilation. The test calls finish(err0) to report pass/fail. Note: the comparison uses && (logical AND) between the two inequality checks, meaning the error is flagged only when both values mismatch simultaneously.",
            "Test Description": "Validates LPDDR4 memory read access in half data bus width mode. The test configures the controller for half bus width, disables DBI and data mask, performs training, then reads pre-initialized data from two consecutive addresses on both port 0 (conditionally) and port 1 (unconditionally). It verifies that the read data matches the expected fixed patterns, confirming correct memory access behavior in half bus width configuration.",
            "Meta Test Steps / Procedure": "1. Call gpv_programming() for bus configuration.\n2. Set err0=0.\n3. Configure port0_addr, port1_addr, ctl_base, phy_base based on APS_DRAM or MPS_DRAM conditional compilation. APS_DRAM: port0_addr=0, port1_addr=0x15A0000000, ctl_base=0x9EE03000, phy_base=0x9F000000. MPS_DRAM: port0_addr=0, port1_addr=0x11A0000000, ctl_base=0x11D003000, phy_base=0x11D500000.\n4. Set bus_width=1 (Half Bus).\n5. Set SG speed grade based on SG2667/SG2133/default(3200) defines.\n6. Set DBI_EN=0, DM_EN=0.\n7. Call lpddr4_training().\n8. Call training_done().\n9. Call wait_on(1000).\n10. Phase 1 (conditional on preprocessor guard: !((APS_DRAM && A53_INITIATOR) ^ (MPS_DRAM && AI_INITIATOR) ^ (MPS_DRAM && DSP_INITIATOR))):\n a. Read port0_addr+0x100 via read_reg64 into read_data0.\n b. Read port0_addr+0x108 via read_reg64 into read_data1.\n c. Check if (read_data0 != 0x3333333333333333) && (read_data1 != 0x2222222222222222). If true, print ERROR with read_data0 and read_data1, increment err0.\n11. Phase 2 (unconditional):\n a. Read port1_addr+0x100 via read_reg64 into read_data2.\n b. Read port1_addr+0x108 via read_reg64 into read_data3.\n c. Check if (read_data2 != 0x3333333333333333) && (read_data3 != 0x2222222222222222). If true, print ERROR with read_data2 and read_data3, increment err0.\n12. Call finish(err0).",
            "Test Steps / Procedure": "1. Perform bus (GPV) programming.\n2. Configure port addresses, controller base, and PHY base for the target subsystem.\n3. Set bus width to half, disable DBI and data mask.\n4. Perform LPDDR4 training and wait for training completion.\n5. (Conditional phase) Read two consecutive 64-bit values from port 0 at fixed addresses and verify they match the expected data patterns.\n6. Read two consecutive 64-bit values from port 1 at the same fixed offsets and verify they match the expected data patterns.\n7. Report test pass or fail based on accumulated error count.",
            "Meta Impacted Registers": "NA",
            "Impacted Registers": "NA",
            "Meta Validation / Acceptance Criteria": "1. Phase 1 port0 read: read_data0 from port0_addr+0x100 must equal 0x3333333333333333 OR read_data1 from port0_addr+0x108 must equal 0x2222222222222222 (error only when both mismatch due to && logic in condition).\n2. Phase 2 port1 read: read_data2 from port1_addr+0x100 must equal 0x3333333333333333 OR read_data3 from port1_addr+0x108 must equal 0x2222222222222222 (error only when both mismatch due to && logic in condition).\n3. err0 must be 0 for test pass via finish(err0).",
            "Validation / Acceptance Criteria": "1. Data read from port 0 at the first address must match the expected pattern, or data at the second address must match its expected pattern (error flagged only when both values mismatch simultaneously).\n2. Data read from port 1 at the first address must match the expected pattern, or data at the second address must match its expected pattern (error flagged only when both values mismatch simultaneously).\n3. The test must complete with zero errors for a pass verdict.",
            "Remarks": "Test uses conditional compilation (APS_DRAM vs MPS_DRAM) for subsystem-specific addresses. Port 0 read phase is additionally guarded by an initiator/subsystem preprocessor condition involving A53_INITIATOR, AI_INITIATOR, and DSP_INITIATOR defines. bus_width is set to 1 for half bus mode. DBI and DM are both disabled. The test does not perform explicit writes; it reads pre-initialized memory patterns, implying data was written during or before training. The error condition uses && (logical AND) between two inequality checks, which means an error is only reported when both read values mismatch their expected patterns simultaneously. No register-access macros or mapped registers were identified by Agent 2 or Agent 4."
        }
    ]

    # Column definitions
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

    # Create workbook
    wb = openpyxl.Workbook()
    ws_tp = wb.active
    ws_tp.title = "TestPlan"
    ws_md = wb.create_sheet("MetaData")

    # Formatting
    header_font = Font(name='Calibri', bold=True, color='FFFFFF', size=11)
    header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
    wrap_alignment = Alignment(wrap_text=True, vertical='top')

    # Write TestPlan headers
    for col_idx, col_name in enumerate(testplan_columns, 1):
        cell = ws_tp.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_alignment

    # Write TestPlan data
    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(testplan_columns, 1):
            value = row_data.get(col_name, "")
            cell = ws_tp.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = wrap_alignment

    # Write MetaData headers
    for col_idx, col_name in enumerate(metadata_columns, 1):
        cell = ws_md.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_alignment

    # Write MetaData data
    for row_idx, row_data in enumerate(json_data, 2):
        for col_idx, col_name in enumerate(metadata_columns, 1):
            value = row_data.get(col_name, "")
            cell = ws_md.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = wrap_alignment

    # Freeze first row
    ws_tp.freeze_panes = 'A2'
    ws_md.freeze_panes = 'A2'

    # Auto-size columns
    MAX_WIDTH = 60
    MIN_WIDTH = 10
    for ws in [ws_tp, ws_md]:
        for col_cells in ws.columns:
            max_length = 0
            col_letter = col_cells[0].column_letter
            for cell in col_cells:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        max_length = max(max_length, len(line))
            adjusted_width = min(max(max_length + 2, MIN_WIDTH), MAX_WIDTH)
            ws.column_dimensions[col_letter].width = adjusted_width

    # Set MetaData sheet to veryHidden
    ws_md.sheet_state = 'veryHidden'

    # Generate filename
    filename = f"LPDDR4_TestPlan_{timestamp}.xlsx"
    output_dir = os.environ.get('OUTPUT_DIR', 'Test_Output/LPDDR4/TestPlan')
    os.makedirs(output_dir, exist_ok=True)
    filepath = os.path.join(output_dir, filename)

    # Save workbook
    wb.save(filepath)

    # Post-save validation
    file_exists = os.path.exists(filepath)
    file_size = os.path.getsize(filepath) if file_exists else 0
    try:
        wb_check = openpyxl.load_workbook(filepath)
        can_reopen = True
        has_testplan = 'TestPlan' in wb_check.sheetnames
        has_metadata = 'MetaData' in wb_check.sheetnames
        tp_rows = wb_check['TestPlan'].max_row - 1
        md_rows = wb_check['MetaData'].max_row - 1
        md_state = wb_check['MetaData'].sheet_state
        wb_check.close()
    except Exception as e:
        can_reopen = False
        has_testplan = False
        has_metadata = False
        tp_rows = 0
        md_rows = 0
        md_state = 'unknown'

    all_passed = file_exists and file_size > 0 and can_reopen and has_testplan and has_metadata

    result = {
        "filename": filename,
        "filepath": filepath,
        "file_size": file_size,
        "rows_testplan": tp_rows,
        "rows_metadata": md_rows,
        "metadata_state": md_state,
        "validation": "PASSED" if all_passed else "FAILED"
    }
    print(json.dumps(result, indent=2))

    # Set output for GitHub Actions
    github_output = os.environ.get('GITHUB_OUTPUT', '')
    if github_output:
        with open(github_output, 'a') as f:
            f.write(f"filename={filename}\n")
            f.write(f"filepath={filepath}\n")
            f.write(f"rows_testplan={tp_rows}\n")
            f.write(f"rows_metadata={md_rows}\n")
            f.write(f"validation={'PASSED' if all_passed else 'FAILED'}\n")

    return 0 if all_passed else 1

if __name__ == '__main__':
    sys.exit(main())
