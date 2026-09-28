#!/usr/bin/env python3
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import os
import sys

# IST timezone
IST = timezone(timedelta(hours=5, minutes=30))
now_ist = datetime.now(IST)
timestamp = now_ist.strftime('%Y%m%d_%H%M%S')
filename = f'LPDDR4_TestPlan_{timestamp}.xlsx'

# Output directory
script_dir = os.path.dirname(os.path.abspath(__file__))
output_path = os.path.join(script_dir, filename)

# JSON data
json_data = [
    {
        "Index": "1",
        "SS / Module": "LPDDR4",
        "Test Case Name": "lpddr4_mem_access_test",
        "Feature": "Memory Access Verification",
        "Meta Headers": '<stdio.h>; <stdlib.h>; \"test_common.h\"; \"lpddr4.h\"',
        "Meta Macros": "NA",
        "Meta Arrays": "exp_data_array[50]",
        "Speed": "SG = 2667; SG = 2133; SG = 3200",
        "Mode": "bus_width = 0 (Full Bus); DBI_EN = 0; DM_EN = 0",
        "Memory Start Offset": "0x0; 0x15A0000000; 0x11A0000000",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase verifies LPDDR4 memory access at multiple data widths (8-bit, 16-bit, 32-bit, 64-bit) for both read and write operations. The test begins by calling gpv_programming() and then configuring base addresses depending on whether APS_DRAM or MPS_DRAM is defined. For APS_DRAM: port0_addr=0, port1_addr=0x15A0000000, ctl_base=0x9EE03000, phy_base=0x9F000000. For MPS_DRAM: port0_addr=0, port1_addr=0x11A0000000, ctl_base=0x11D003000, phy_base=0x11D500000. bus_width is set to 0 (Full Bus), DBI_EN=0, DM_EN=0. Speed grade (SG) is set conditionally: 2667 for SG2667, 2133 for SG2133, or 3200 by default. lpddr4_training() is called to initialize the memory. The check_mem_access() function is called for port0_addr (conditionally based on initiator/DRAM configuration) and for port1_addr + 0x100000. Inside check_mem_access(), a random 64-bit data value is generated using rand(). The data is written to the address using write_reg64(addr, data). Then the data is read back using read_reg8 (8 iterations for each byte), read_reg16 (4 iterations for each half-word), read_reg (2 iterations for each word), and read_reg64 (one 64-bit read). Each read-back value is compared against the corresponding portion of the original data using pointer casting (ptr8, ptr16, ptr32, ptr64). Any mismatch increments err0. A second phase at addr + 0x1000 generates new random data and performs mixed-width writes: write_reg8 for bytes 0 and 1, write_reg16 for half-word at offset 2, write_reg for word at offset 4. Then read_reg64 reads back the full 64-bit value and compares against the expected data. finish(err0) is called at the end to report pass/fail.",
        "Test Description": "This test verifies LPDDR4 memory access correctness across multiple data widths. It initializes the LPDDR4 controller and PHY, performs training, then writes a random 64-bit data pattern to a memory address. The test reads back the data using 8-bit, 16-bit, 32-bit, and 64-bit access widths and verifies each read matches the corresponding portion of the written data. A second verification phase writes data using mixed access widths (two 8-bit writes, one 16-bit write, one 32-bit write) to a different offset and reads back the full 64-bit value to confirm correctness. The test is performed on both port0 and port1 memory regions, with the configuration varying based on the DRAM subsystem (APS or MPS) and initiator type.",
        "Meta Test Steps / Procedure": "1. Call gpv_programming() for GPV configuration.\n2. Set port0_addr = 0, and conditionally set port1_addr, ctl_base, phy_base based on APS_DRAM or MPS_DRAM:\n   - APS_DRAM: port1_addr=0x15A0000000, ctl_base=0x9EE03000, phy_base=0x9F000000\n   - MPS_DRAM: port1_addr=0x11A0000000, ctl_base=0x11D003000, phy_base=0x11D500000\n3. Set bus_width=0 (Full Bus), DBI_EN=0, DM_EN=0.\n4. Set SG (speed grade) conditionally: SG2667->2667, SG2133->2133, default->3200.\n5. Call lpddr4_training() to perform LPDDR4 training.\n6. Conditionally call check_mem_access(port0_addr) based on initiator/DRAM configuration macro combination.\n7. Call check_mem_access(port1_addr + 0x100000).\n8. Inside check_mem_access(addr):\n   a. Generate random 64-bit data: data = rand() | ((unsigned long)rand()<<32).\n   b. Set ptr8, ptr16, ptr32, ptr64 pointers to &data.\n   c. Write 64-bit data: write_reg64(addr, data).\n   d. Read back 8 bytes using read_reg8(addr+i) for i=0..7, compare each byte with *(ptr8+i), increment err0 on mismatch.\n   e. Read back 4 half-words using read_reg16(addr+(i*2)) for i=0..3, compare each with *(ptr16+i), increment err0 on mismatch.\n   f. Read back 2 words using read_reg(addr+(i*4)) for i=0..1, compare each with *(ptr32+i), increment err0 on mismatch.\n   g. Read back 64-bit value using read_reg64(addr, &data64), compare with *ptr64, increment err0 on mismatch.\n   h. Advance address: addr = addr + 0x1000.\n   i. Generate new random 64-bit data.\n   j. Perform mixed-width writes: write_reg8(addr+0, byte0), write_reg8(addr+1, byte1), write_reg16(addr+2, halfword1), write_reg(addr+4, word1).\n   k. Read back 64-bit value using read_reg64(addr, &data64), compare with expected data, print error on mismatch.\n9. Call finish(err0) to report test result.",
        "Test Steps / Procedure": "1. Perform GPV programming initialization.\n2. Configure LPDDR4 controller and PHY base addresses based on the target DRAM subsystem (APS or MPS).\n3. Set data bus width to full bus mode, disable DBI and DM.\n4. Set the speed grade based on the target configuration.\n5. Execute LPDDR4 memory training.\n6. For each applicable memory port address, perform the following memory access verification:\n   a. Write a random 64-bit data pattern to the target memory address.\n   b. Read back the data using 8-bit access width (8 individual byte reads) and verify each byte matches the corresponding byte of the written data.\n   c. Read back the data using 16-bit access width (4 half-word reads) and verify each half-word matches.\n   d. Read back the data using 32-bit access width (2 word reads) and verify each word matches.\n   e. Read back the data using 64-bit access width and verify the full value matches.\n7. At a secondary offset, write data using mixed access widths (two byte writes, one half-word write, one word write) and read back the full 64-bit value to verify correctness.\n8. Report test pass or fail based on the accumulated error count.",
        "Meta Impacted Registers": "NA",
        "Impacted Registers": "NA",
        "Meta Validation / Acceptance Criteria": "For each read-back operation in check_mem_access():\n- 8-bit reads: data8 from read_reg8(addr+i) must equal *(ptr8+i) for i=0..7. Mismatch increments err0.\n- 16-bit reads: data16 from read_reg16(addr+(i*2)) must equal *(ptr16+i) for i=0..3. Mismatch increments err0.\n- 32-bit reads: data32 from read_reg(addr+(i*4)) must equal *(ptr32+i) for i=0..1. Mismatch increments err0.\n- 64-bit read: data64 from read_reg64(addr, &data64) must equal *ptr64. Mismatch increments err0.\n- Mixed-width write verification: After writing bytes, half-word, and word individually, read_reg64(addr, &data64) must return data64 equal to the full expected 64-bit data value.\n- Final pass/fail: finish(err0) is called. Test passes if err0 == 0 (no mismatches detected).",
        "Validation / Acceptance Criteria": "1. All 8-bit read-back values must match the corresponding bytes of the originally written 64-bit data pattern.\n2. All 16-bit read-back values must match the corresponding half-words of the written data.\n3. All 32-bit read-back values must match the corresponding words of the written data.\n4. The 64-bit read-back value must exactly match the full written 64-bit data pattern.\n5. After performing mixed-width writes (byte, half-word, and word writes to consecutive offsets), the 64-bit read-back must match the expected composite data value.\n6. The test passes if no mismatches are detected across all access width verifications (error count is zero).",
        "Remarks": "Test uses conditional compilation to support both APS and MPS DRAM subsystems with different base addresses and initiator configurations. Port0 access is conditionally skipped based on specific initiator/DRAM macro combinations. Random data patterns are generated using rand() for test coverage. The test validates data integrity across all standard access widths (8, 16, 32, 64 bits) and mixed-width write scenarios. No direct register-access macros or hardcoded register addresses were identified as register operands in register access calls; all memory operations use runtime variable addresses."
    },
    {
        "Index": "2",
        "SS / Module": "LPDDR4",
        "Test Case Name": "lpddr4_mem_basic_wr_rd_test",
        "Feature": "Basic Memory Write-Read Verification",
        "Meta Headers": '<stdio.h>; <stdlib.h>; \"test_common.h\"; \"lpddr4.h\"',
        "Meta Macros": "NA",
        "Meta Arrays": "exp_data_array[50]",
        "Speed": "SG = 2667; SG = 2133; SG = 3200",
        "Mode": "bus_width = 0 (Full Bus); DBI_EN = 0; DM_EN = 0",
        "Memory Start Offset": "0x0; 0x15A0000000; 0x11A0000000",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase verifies basic LPDDR4 memory write and read operations using 64-bit data patterns across two memory ports. The test begins by calling gpv_programming() and configuring base addresses depending on whether APS_DRAM or MPS_DRAM is defined. For APS_DRAM: port0_addr=0, port1_addr=0x15A0000000, ctl_base=0x9EE03000, phy_base=0x9F000000. For MPS_DRAM: port0_addr=0, port1_addr=0x11A0000000, ctl_base=0x11D003000, phy_base=0x11D500000. bus_width is set to 0 (Full Bus), DBI_EN=0, DM_EN=0. Speed grade (SG) is set conditionally: 2667 for SG2667, 2133 for SG2133, or 3200 by default. lpddr4_training() is called to initialize the memory. Phase 1 (conditionally compiled based on initiator/DRAM macro combination): offset is set to 0x1000. A loop writes 10 random 64-bit values to port0_addr + (index * 0x1000) using write_reg64(). Random data is generated as rand() | ((unsigned long)rand()<<32) and stored in exp_data_array[index]. A second loop reads back from port0_addr + (index * 0x1000) using read_reg64() and compares read_data with exp_data_array[index]; mismatch increments err0. A third loop reads back from port1_addr + (index * 0x1000) using read_reg64() and compares with the same exp_data_array[index]; mismatch increments err0. Phase 2 (unconditional): offset is set to 0x20000000. A loop writes 32 random 64-bit values to port1_addr + (index * 0x20000000) using write_reg64(). A second loop reads back from port1_addr + (index * 0x20000000) using read_reg64() and compares read_data with exp_data_array[index]; mismatch increments err0. finish(err0) is called at the end to report pass/fail.",
        "Test Description": "This test verifies basic LPDDR4 memory write and read correctness using 64-bit data patterns. It initializes the LPDDR4 controller and PHY, performs training, then executes two verification phases. In the first phase, 10 random 64-bit values are written to port0 memory at regular intervals, then read back from both port0 and port1 to verify data integrity and port mirroring. In the second phase, 32 random 64-bit values are written to port1 memory at larger address intervals spanning a wide address range, then read back and verified. The test covers both APS and MPS DRAM subsystem configurations and reports pass or fail based on the total number of mismatches detected.",
        "Meta Test Steps / Procedure": "1. Call gpv_programming() for GPV configuration.\n2. Set port0_addr = 0, and conditionally set port1_addr, ctl_base, phy_base based on APS_DRAM or MPS_DRAM:\n   - APS_DRAM: port1_addr=0x15A0000000, ctl_base=0x9EE03000, phy_base=0x9F000000\n   - MPS_DRAM: port1_addr=0x11A0000000, ctl_base=0x11D003000, phy_base=0x11D500000\n3. Set bus_width=0 (Full Bus), DBI_EN=0, DM_EN=0.\n4. Set SG (speed grade) conditionally: SG2667->2667, SG2133->2133, default->3200.\n5. Call lpddr4_training() to perform LPDDR4 training.\n6. Phase 1 (conditional on initiator/DRAM macro combination):\n   a. Set offset = 0x1000.\n   b. Loop index=0..9: Generate random 64-bit data, store in exp_data_array[index], write to port0_addr + (index * offset) using write_reg64().\n   c. Loop index=0..9: Read from port0_addr + (index * offset) using read_reg64(&read_data). Compare read_data with exp_data_array[index]. If mismatch, print ERROR_0 for port0 and increment err0.\n   d. Loop index=0..9: Read from port1_addr + (index * offset) using read_reg64(&read_data). Compare read_data with exp_data_array[index]. If mismatch, print ERROR_0 for port1 and increment err0.\n7. Phase 2 (unconditional):\n   a. Set offset = 0x20000000.\n   b. Loop index=0..31: Generate random 64-bit data, store in exp_data_array[index], write to port1_addr + (index * offset) using write_reg64().\n   c. Loop index=0..31: Read from port1_addr + (index * offset) using read_reg64(&read_data). Compare read_data with exp_data_array[index]. If mismatch, print ERROR_1 for port1 and increment err0.\n8. Call finish(err0) to report test result.",
        "Test Steps / Procedure": "1. Perform GPV programming initialization.\n2. Configure LPDDR4 controller and PHY base addresses based on the target DRAM subsystem (APS or MPS).\n3. Set data bus width to full bus mode, disable DBI and DM.\n4. Set the speed grade based on the target configuration.\n5. Execute LPDDR4 memory training.\n6. Phase 1 - Port0 write and dual-port read-back verification:\n   a. Write 10 random 64-bit data patterns to port0 memory at regular address intervals.\n   b. Read back all 10 locations from port0 and verify each value matches the written data.\n   c. Read back all 10 locations from port1 at the same offsets and verify each value matches the written data.\n7. Phase 2 - Port1 wide-range write and read-back verification:\n   a. Write 32 random 64-bit data patterns to port1 memory at large address intervals spanning a wide address range.\n   b. Read back all 32 locations from port1 and verify each value matches the written data.\n8. Report test pass or fail based on the accumulated error count.",
        "Meta Impacted Registers": "NA",
        "Impacted Registers": "NA",
        "Meta Validation / Acceptance Criteria": "Phase 1 validation:\n- For port0 read-back: read_data from read_reg64(port0_addr + (index * 0x1000)) must equal exp_data_array[index] for index=0..9. Mismatch prints ERROR_0 for port0 and increments err0.\n- For port1 read-back: read_data from read_reg64(port1_addr + (index * 0x1000)) must equal exp_data_array[index] for index=0..9. Mismatch prints ERROR_0 for port1 and increments err0.\n\nPhase 2 validation:\n- For port1 read-back: read_data from read_reg64(port1_addr + (index * 0x20000000)) must equal exp_data_array[index] for index=0..31. Mismatch prints ERROR_1 for port1 and increments err0.\n\nFinal pass/fail: finish(err0) is called. Test passes if err0 == 0 (no mismatches detected across all phases).",
        "Validation / Acceptance Criteria": "1. In Phase 1, all 10 read-back values from port0 must exactly match the corresponding written 64-bit data patterns.\n2. In Phase 1, all 10 read-back values from port1 at the same offsets must exactly match the data originally written to port0, verifying cross-port data consistency.\n3. In Phase 2, all 32 read-back values from port1 across the wide address range must exactly match the corresponding written 64-bit data patterns.\n4. The test passes if no mismatches are detected across all read-back verifications in both phases (error count is zero).",
        "Remarks": "Test uses conditional compilation to support both APS and MPS DRAM subsystems with different base addresses. Phase 1 port0 write and dual-port read-back is conditionally compiled based on specific initiator/DRAM macro combinations using an XOR expression. Phase 2 uses a large offset of 0x20000000 between writes to exercise a wide memory address range across 32 entries. Random data patterns are generated using rand() combined with a 32-bit left shift to produce full 64-bit values. No direct register-access macros or hardcoded register addresses were identified as register operands in register access calls; all memory operations use runtime variable addresses."
    },
    {
        "Index": "3",
        "SS / Module": "LPDDR4",
        "Test Case Name": "lpddr4_mem_boundary_test",
        "Feature": "Memory Boundary Verification",
        "Meta Headers": '<stdio.h>; <stdlib.h>; \"test_common.h\"; \"lpddr4.h\"',
        "Meta Macros": "NA",
        "Meta Arrays": "exp_data_array[150]",
        "Speed": "SG = 2667; SG = 2133; SG = 3200",
        "Mode": "bus_width = 0 (Full Bus); DBI_EN = 0; DM_EN = 0",
        "Memory Start Offset": "0x0; 0x15A0000000; 0x11A0000000",
        "Memory End Offset": "val_2g = 0x0000000080000000; val_16g = 0x0000000400000000",
        "Meta Test Description": "This testcase verifies LPDDR4 memory data integrity at boundary address ranges up to 2GB and 16GB limits. The test begins by calling gpv_programming() and configuring base addresses depending on whether APS_DRAM or MPS_DRAM is defined. For APS_DRAM: port0_addr=0, port1_addr=0x15A0000000, ctl_base=0x9EE03000, phy_base=0x9F000000. For MPS_DRAM: port0_addr=0, port1_addr=0x11A0000000, ctl_base=0x11D003000, phy_base=0x11D500000. bus_width is set to 0 (Full Bus), DBI_EN=0, DM_EN=0. Speed grade (SG) is set conditionally: 2667 for SG2667, 2133 for SG2133, or 3200 by default. lpddr4_training() is called to initialize the memory. Two boundary values are defined: val_2g = 0x0000000080000000 (2GB) and val_16g = 0x0000000400000000 (16GB). Phase 1 (conditionally compiled based on initiator/DRAM macro combination): offset is set to PORT0_OFF if defined, otherwise 0x1000000. A loop writes random 64-bit values to port0_addr + (index1 * offset) using write_reg64() for all index1 where (index1 * offset) < val_2g. Random data is generated as rand() | ((unsigned long)rand()<<32) and stored in exp_data_array[index1]. A second loop reads back from port0_addr + (index1 * offset) using read_reg64() and compares read_data with exp_data_array[index1]; mismatch increments err0. A third loop reads back from port1_addr + (index1 * offset) using read_reg64() and compares with the same exp_data_array[index1]; mismatch increments err0. Phase 2 (unconditional): offset is set to PORT1_OFF if defined, otherwise 0x08000000. A loop writes random 64-bit values to port1_addr + (index1 * offset) using write_reg64() for all index1 where (index1 * offset) < val_16g. A second loop reads back from port1_addr + (index1 * offset) using read_reg64() and compares read_data with exp_data_array[index1]; mismatch increments err0. finish(err0) is called at the end to report pass/fail.",
        "Test Description": "This test verifies LPDDR4 memory data integrity across boundary address ranges up to 2GB and 16GB limits. It initializes the LPDDR4 controller and PHY, performs training, then executes two boundary verification phases. In the first phase, random 64-bit data patterns are written to port0 memory at regular intervals spanning the entire 2GB address range, then read back from both port0 and port1 to verify data integrity and cross-port consistency. In the second phase, random 64-bit data patterns are written to port1 memory at larger intervals spanning the full 16GB address range, then read back and verified. The test ensures memory is accessible and data-correct across the full supported address space for both APS and MPS DRAM subsystem configurations.",
        "Meta Test Steps / Procedure": "1. Call gpv_programming() for GPV configuration.\n2. Set port0_addr = 0, and conditionally set port1_addr, ctl_base, phy_base based on APS_DRAM or MPS_DRAM:\n   - APS_DRAM: port1_addr=0x15A0000000, ctl_base=0x9EE03000, phy_base=0x9F000000\n   - MPS_DRAM: port1_addr=0x11A0000000, ctl_base=0x11D003000, phy_base=0x11D500000\n3. Set bus_width=0 (Full Bus), DBI_EN=0, DM_EN=0.\n4. Set SG (speed grade) conditionally: SG2667->2667, SG2133->2133, default->3200.\n5. Define boundary values: val_2g = 0x0000000080000000, val_16g = 0x0000000400000000.\n6. Call lpddr4_training() to perform LPDDR4 training.\n7. Phase 1 (conditional on initiator/DRAM macro combination):\n   a. Set offset = PORT0_OFF if defined, else offset = 0x1000000.\n   b. Loop index1 while (index1*offset) < val_2g: Generate random 64-bit data, store in exp_data_array[index1], write to port0_addr + (index1 * offset) using write_reg64().\n   c. Loop index1 while (index1*offset) < val_2g: Read from port0_addr + (index1 * offset) using read_reg64(&read_data). Compare read_data with exp_data_array[index1]. If mismatch, print ERROR_0 for port0 and increment err0.\n   d. Loop index1 while (index1*offset) < val_2g: Read from port1_addr + (index1 * offset) using read_reg64(&read_data). Compare read_data with exp_data_array[index1]. If mismatch, print ERROR_0 for port1 and increment err0.\n8. Phase 2 (unconditional):\n   a. Set offset = PORT1_OFF if defined, else offset = 0x08000000.\n   b. Loop index1 while (index1*offset) < val_16g: Generate random 64-bit data, store in exp_data_array[index1], write to port1_addr + (index1 * offset) using write_reg64().\n   c. Loop index1 while (index1*offset) < val_16g: Read from port1_addr + (index1 * offset) using read_reg64(&read_data). Compare read_data with exp_data_array[index1]. If mismatch, print ERROR_1 for port1 and increment err0.\n9. Call finish(err0) to report test result.",
        "Test Steps / Procedure": "1. Perform GPV programming initialization.\n2. Configure LPDDR4 controller and PHY base addresses based on the target DRAM subsystem (APS or MPS).\n3. Set data bus width to full bus mode, disable DBI and DM.\n4. Set the speed grade based on the target configuration.\n5. Execute LPDDR4 memory training.\n6. Phase 1 - Port0 boundary write and dual-port read-back verification up to 2GB:\n   a. Write random 64-bit data patterns to port0 memory at regular intervals, iterating until the address offset reaches the 2GB boundary.\n   b. Read back all written locations from port0 and verify each value matches the written data.\n   c. Read back all written locations from port1 at the same offsets and verify each value matches the written data.\n7. Phase 2 - Port1 boundary write and read-back verification up to 16GB:\n   a. Write random 64-bit data patterns to port1 memory at larger intervals, iterating until the address offset reaches the 16GB boundary.\n   b. Read back all written locations from port1 and verify each value matches the written data.\n8. Report test pass or fail based on the accumulated error count.",
        "Meta Impacted Registers": "NA",
        "Impacted Registers": "NA",
        "Meta Validation / Acceptance Criteria": "Phase 1 validation:\n- For port0 read-back: read_data from read_reg64(port0_addr + (index1 * offset)) must equal exp_data_array[index1] for all index1 where (index1*offset) < val_2g. Mismatch prints ERROR_0 for port0 and increments err0.\n- For port1 read-back: read_data from read_reg64(port1_addr + (index1 * offset)) must equal exp_data_array[index1] for all index1 where (index1*offset) < val_2g. Mismatch prints ERROR_0 for port1 and increments err0.\n\nPhase 2 validation:\n- For port1 read-back: read_data from read_reg64(port1_addr + (index1 * offset)) must equal exp_data_array[index1] for all index1 where (index1*offset) < val_16g. Mismatch prints ERROR_1 for port1 and increments err0.\n\nFinal pass/fail: finish(err0) is called. Test passes if err0 == 0 (no mismatches detected across all phases).",
        "Validation / Acceptance Criteria": "1. In Phase 1, all read-back values from port0 across the 2GB address range must exactly match the corresponding written 64-bit data patterns.\n2. In Phase 1, all read-back values from port1 at the same offsets across the 2GB range must exactly match the data originally written to port0, verifying cross-port data consistency.\n3. In Phase 2, all read-back values from port1 across the 16GB address range must exactly match the corresponding written 64-bit data patterns.\n4. The test passes if no mismatches are detected across all read-back verifications in both phases (error count is zero).",
        "Remarks": "Test uses conditional compilation to support both APS and MPS DRAM subsystems with different base addresses. Phase 1 port0 write and dual-port read-back is conditionally compiled based on specific initiator/DRAM macro combinations using an XOR expression. The loop boundaries are determined by boundary values val_2g (2GB) and val_16g (16GB), making the number of iterations dependent on the configured offset. The offset for port0 can be overridden via PORT0_OFF macro (default 0x1000000), and the offset for port1 can be overridden via PORT1_OFF macro (default 0x08000000). The exp_data_array is sized at 150 entries to accommodate the maximum number of iterations. Random data patterns are generated using rand() combined with a 32-bit left shift to produce full 64-bit values. No direct register-access macros or hardcoded register addresses were identified as register operands in register access calls; all memory operations use runtime variable addresses."
    },
    {
        "Index": "4",
        "SS / Module": "LPDDR4",
        "Test Case Name": "lpddr4_mem_quarter_data_bus_width_test",
        "Feature": "Quarter Data Bus Width Verification",
        "Meta Headers": '<stdio.h>; <stdlib.h>; \"test_common.h\"; \"lpddr4.h\"',
        "Meta Macros": "NA",
        "Meta Arrays": "NA",
        "Speed": "SG = 2667; SG = 2133; SG = 3200",
        "Mode": "bus_width = 2 (Quarter bus); DBI_EN = 0; DM_EN = 0",
        "Memory Start Offset": "0x0; 0x15A0000000; 0x11A0000000",
        "Memory End Offset": "NA",
        "Meta Test Description": "This testcase verifies LPDDR4 memory read-back correctness when operating in quarter data bus width mode. The test begins by calling gpv_programming() and configuring base addresses depending on whether APS_DRAM or MPS_DRAM is defined. For APS_DRAM: port0_addr=0, port1_addr=0x15A0000000, ctl_base=0x9EE03000, phy_base=0x9F000000. For MPS_DRAM: port0_addr=0, port1_addr=0x11A0000000, ctl_base=0x11D003000, phy_base=0x11D500000. bus_width is set to 2 (Quarter bus), DBI_EN=0, DM_EN=0. Speed grade (SG) is set conditionally: 2667 for SG2667, 2133 for SG2133, or 3200 by default. lpddr4_training() is called to initialize the memory, followed by training_done() and wait_on(1000). For port0 (conditionally compiled based on initiator/DRAM macro combination using XOR expression): read_reg64(port0_addr + 0x100, &read_data0) reads a 64-bit value at offset 0x100, and read_reg64(port0_addr + 0x108, &read_data1) reads a 64-bit value at offset 0x108. The test checks if read_data0 != 0x3333333333333333 AND read_data1 != 0x2222222222222222; if both conditions are true, an error is printed and err0 is incremented. For port1 (unconditional): read_reg64(port1_addr + 0x100, &read_data2) and read_reg64(port1_addr + 0x108, &read_data3) read 64-bit values at offsets 0x100 and 0x108 respectively. The test checks if read_data2 != 0x3333333333333333 AND read_data3 != 0x2222222222222222; if both conditions are true, an error is printed and err0 is incremented. finish(err0) is called at the end to report pass/fail.",
        "Test Description": "This test verifies LPDDR4 memory data integrity when operating in quarter data bus width mode. It initializes the LPDDR4 controller and PHY with the bus width configured to quarter mode, performs training, waits for training completion, then reads back pre-defined data patterns from two memory locations on each port. The test verifies that the read-back values at two specific offsets match the expected fixed data patterns for both port0 and port1. This validates that the LPDDR4 memory subsystem correctly handles data transfers in quarter bus width configuration for both APS and MPS DRAM subsystems.",
        "Meta Test Steps / Procedure": "1. Call gpv_programming() for GPV configuration.\n2. Set port0_addr = 0, and conditionally set port1_addr, ctl_base, phy_base based on APS_DRAM or MPS_DRAM:\n   - APS_DRAM: port1_addr=0x15A0000000, ctl_base=0x9EE03000, phy_base=0x9F000000\n   - MPS_DRAM: port1_addr=0x11A0000000, ctl_base=0x11D003000, phy_base=0x11D500000\n3. Set bus_width=2 (Quarter bus), DBI_EN=0, DM_EN=0.\n4. Set SG (speed grade) conditionally: SG2667->2667, SG2133->2133, default->3200.\n5. Call lpddr4_training() to perform LPDDR4 training.\n6. Call training_done() to confirm training completion.\n7. Call wait_on(1000) to wait 1000 time units.\n8. Port0 verification (conditional):\n   a. read_reg64(port0_addr + 0x100, &read_data0).\n   b. read_reg64(port0_addr + 0x108, &read_data1).\n   c. Check if (read_data0 != 0x3333333333333333) && (read_data1 != 0x2222222222222222). If true, print ERROR, increment err0.\n9. Port1 verification (unconditional):\n   a. read_reg64(port1_addr + 0x100, &read_data2).\n   b. read_reg64(port1_addr + 0x108, &read_data3).\n   c. Check if (read_data2 != 0x3333333333333333) && (read_data3 != 0x2222222222222222). If true, print ERROR, increment err0.\n10. Call finish(err0) to report test result.",
        "Test Steps / Procedure": "1. Perform GPV programming initialization.\n2. Configure LPDDR4 controller and PHY base addresses based on the target DRAM subsystem (APS or MPS).\n3. Set data bus width to quarter bus mode, disable DBI and DM.\n4. Set the speed grade based on the target configuration.\n5. Execute LPDDR4 memory training and wait for training completion.\n6. Wait for a stabilization period after training.\n7. For port0 (conditionally based on initiator/DRAM configuration), read two 64-bit values from consecutive memory offsets and verify they match the expected fixed data patterns.\n8. For port1 (unconditionally), read two 64-bit values from the same consecutive memory offsets and verify they match the expected fixed data patterns.\n9. Report test pass or fail based on the accumulated error count.",
        "Meta Impacted Registers": "NA",
        "Impacted Registers": "NA",
        "Meta Validation / Acceptance Criteria": "Port0 validation (conditional):\n- read_data0 from read_reg64(port0_addr + 0x100) should equal 0x3333333333333333.\n- read_data1 from read_reg64(port0_addr + 0x108) should equal 0x2222222222222222.\n- Error condition: if (read_data0 != 0x3333333333333333) && (read_data1 != 0x2222222222222222), then ERROR is printed and err0 is incremented.\n\nPort1 validation (unconditional):\n- read_data2 from read_reg64(port1_addr + 0x100) should equal 0x3333333333333333.\n- read_data3 from read_reg64(port1_addr + 0x108) should equal 0x2222222222222222.\n- Error condition: if (read_data2 != 0x3333333333333333) && (read_data3 != 0x2222222222222222), then ERROR is printed and err0 is incremented.\n\nFinal pass/fail: finish(err0) is called. Test passes if err0 == 0 (no mismatches detected).",
        "Validation / Acceptance Criteria": "1. For port0, the first 64-bit read-back value at the first offset must match the expected pattern, and the second 64-bit read-back value at the next offset must match its expected pattern.\n2. For port1, the first 64-bit read-back value at the first offset must match the expected pattern, and the second 64-bit read-back value at the next offset must match its expected pattern.\n3. The error condition triggers only when both read values on a given port simultaneously fail to match their respective expected patterns.\n4. The test passes if no errors are detected across both port verifications (error count is zero).",
        "Remarks": "Test uses conditional compilation to support both APS and MPS DRAM subsystems with different base addresses. Port0 verification is conditionally compiled based on specific initiator/DRAM macro combinations using an XOR expression. The bus_width is set to 2 indicating quarter bus mode, which is the primary feature under test. Unlike other LPDDR4 testcases in this suite, this test does not write data before reading. The validation uses an AND condition for the error check, meaning an error is flagged only if both read values simultaneously differ from their expected patterns. A wait_on(1000) delay is inserted after training completion before read verification begins."
    }
]

# TestPlan columns
tp_columns = [
    'Index', 'SS / Module', 'Feature', 'Test Case Name', 'Test Description',
    'Speed', 'Mode', 'Memory Start Offset', 'Memory End Offset', 'Remarks',
    'Test Steps / Procedure', 'Impacted Registers', 'Validation / Acceptance Criteria',
    'Code Generation'
]

# MetaData columns
md_columns = [
    'Index', 'Test Case Name', 'Meta Test Description', 'Meta Test Steps / Procedure',
    'Meta Impacted Registers', 'Meta Validation / Acceptance Criteria',
    'Meta Headers', 'Meta Macros', 'Meta Arrays'
]

# Create workbook
wb = openpyxl.Workbook()

# TestPlan sheet
ws_tp = wb.active
ws_tp.title = 'TestPlan'

# MetaData sheet
ws_md = wb.create_sheet('MetaData')

# Styles
header_font = Font(bold=True, color='FFFFFF', size=11)
header_fill = PatternFill(start_color='4472C4', end_color='4472C4', fill_type='solid')
wrap_alignment = Alignment(wrap_text=True, vertical='top')

def populate_sheet(ws, columns, data, is_testplan=False):
    # Write headers
    for col_idx, col_name in enumerate(columns, 1):
        cell = ws.cell(row=1, column=col_idx, value=col_name)
        cell.font = header_font
        cell.fill = header_fill
        cell.alignment = wrap_alignment
    
    # Write data rows
    for row_idx, row_data in enumerate(data, 2):
        for col_idx, col_name in enumerate(columns, 1):
            if col_name == 'Code Generation':
                value = ''
            else:
                value = row_data.get(col_name, '')
            cell = ws.cell(row=row_idx, column=col_idx, value=value)
            cell.alignment = wrap_alignment
    
    # Auto-size columns
    for col_idx, col_name in enumerate(columns, 1):
        max_length = len(str(col_name))
        for row in ws.iter_rows(min_row=2, min_col=col_idx, max_col=col_idx):
            for cell in row:
                if cell.value:
                    lines = str(cell.value).split('\n')
                    for line in lines:
                        if len(line) > max_length:
                            max_length = len(line)
        adjusted_width = min(max_length + 2, 60)
        ws.column_dimensions[openpyxl.utils.get_column_letter(col_idx)].width = adjusted_width
    
    # Freeze first row
    ws.freeze_panes = 'A2'

# Populate sheets
populate_sheet(ws_tp, tp_columns, json_data, is_testplan=True)
populate_sheet(ws_md, md_columns, json_data)

# Set MetaData sheet to veryHidden
ws_md.sheet_state = 'veryHidden'

# Save workbook
wb.save(output_path)

# Validation
try:
    wb2 = openpyxl.load_workbook(output_path)
    sheets = wb2.sheetnames
    assert 'TestPlan' in sheets, 'TestPlan sheet missing'
    assert 'MetaData' in sheets, 'MetaData sheet missing'
    tp_rows = wb2['TestPlan'].max_row - 1
    md_rows = wb2['MetaData'].max_row - 1
    file_size = os.path.getsize(output_path)
    assert file_size > 0, 'File is empty'
    print(f'VALIDATION PASSED')
    print(f'Filename: {filename}')
    print(f'Path: {output_path}')
    print(f'Size: {file_size} bytes')
    print(f'TestPlan rows: {tp_rows}')
    print(f'MetaData rows: {md_rows}')
    print(f'Sheets: {sheets}')
except Exception as e:
    print(f'VALIDATION FAILED: {e}')
    sys.exit(1)
