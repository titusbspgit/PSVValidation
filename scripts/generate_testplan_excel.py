#!/usr/bin/env python3
import os
import json
from datetime import datetime, timezone, timedelta
from typing import List, Dict, Any

from openpyxl import Workbook
from openpyxl.styles import Font

# 1) Load JSON data embedded below exactly as provided
JSON_DATA: List[Dict[str, Any]] = json.loads(r'''[
  {
    "Index": "1",
    "SS / Module": "PCIE",
    "Feature": "Testable: writeAsRead",
    "Test Case Name": "pcie0_dbi_dsp_reg_wr_rd_test",
    "Test Description": "Validate PCIe0 DBI/DSP configuration and BAR registers by checking default reset values where readable, then performing masked write/read using multiple data patterns to confirm writable bits update while read-only bits retain defaults. Non-readable/writable and explicitly excluded registers are skipped. Pass if no mismatches are observed.",
    "Meta Test Description": "The test iterates over a predefined PCIe0 DBI/DSP register address list (addr_array). Phase 1 (Default-Value Check): For each index i in [0..CNT-1], if read_mask_array[i] == 0, the address is skipped as not readable. Additionally, three addresses (DBI_DSP_CAP_ID_NXT_PTR_REG, DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, DBI_DSP_PL_DEBUG1_OFF) are explicitly excluded from default-value checking. For other entries, read_reg(addr_array[i]) is compared with default_value_array[i]; on mismatch, def_fail_cnt is incremented and a failure message is printed. Phase 2 (Write/Read Masked Verification): For each data pattern in {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000}, the test writes data_wr to each address if (skip_array[i] != 1) and (write_mask_array[i] != 0). Then it reads back each address if not skipped, writable, and readable. The expected value is computed as exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((~write_mask_array[i]) & read_mask_array[i] & default_value_array[i])). If data_rd != exp_val, wr_fail_cnt is incremented and a failure is logged. Upon completion, finish(1) is called if def_fail_cnt > 0 or wr_fail_cnt > 0; else finish(0). A soft-reset helper (soft_reset_chk) exists but is not executed in this flow (commented out).",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Default-value check intentionally skips DBI_DSP_CAP_ID_NXT_PTR_REG, DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, and DBI_DSP_PL_DEBUG1_OFF. Registers marked non-readable or non-writable via masks are skipped. Only writable fields are updated during pattern writes; read-only fields are expected to retain their reset defaults.",
    "Test Steps / Procedure": "1) Initialize the test environment and obtain the PCIe0 DBI/DSP register list (including Type1 Device/Vendor ID, Status/Command, Class Code/Revision, Header/BIST fields, BAR0, BAR1, secondary bus timing and I/O limits, memory and prefetchable memory limits). 2) For each listed register that is readable, read the current value and compare it against its defined reset default; skip the explicitly excluded registers and any marked non-readable. 3) For each data pattern (all-ones, alternating A, alternating 5, all-zeros, A5 pattern, upper-half ones), write the pattern to each register that is designated writable and not skipped. 4) Read back each written register that is both writable and readable; verify that only the writable bits reflect the pattern and that read-only bits match their default values per the masks. 5) Aggregate any mismatches from default checks and write/read verification; declare the test PASS only if no mismatches are found across all registers and patterns.",
    "Meta Test Steps / Procedure": "1) Call chk_rst_val(): For i in [0..CNT-1]: addr = addr_array[i]; if read_mask_array[i] == 0x00000000 → continue; if addr == mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG || addr == mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS || addr == mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF → continue; data_rd = read_reg(addr); if (data_rd == default_value_array[i]) → PASS log; else { def_fail_cnt++; FAIL log }. 2) Call chk_rd_wr(): Define chk_val[6] = {0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}. For each j in [0..5]: data_wr = chk_val[j]; 2a) Write phase: For i in [0..CNT-1]: addr = addr_array[i]; if skip_array[i] == 1 → continue; if write_mask_array[i] == 0x00000000 → continue; else write_reg(addr, data_wr). 2b) Read/verify phase: For i in [0..CNT-1]: addr = addr_array[i]; if skip_array[i] == 1 → continue; if write_mask_array[i] == 0x00000000 → continue; if read_mask_array[i] == 0x00000000 → continue; data_rd = read_reg(addr); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd == exp_val) → PASS log; else { wr_fail_cnt++; FAIL log }. 3) After executing chk_rst_val() and chk_rd_wr(), if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0). Note: soft_reset_chk() (writes SOFT_RST_REG_ADDRESS with SOFT_RST_REG_DATA, waits, restores default) is present but not executed in the main flow.",
    "Impacted Registers": "DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG, DBI_DSP_TYPE1_STATUS_COMMAND_REG, DBI_DSP_TYPE1_CLASS_CODE_REV_ID_REG, DBI_DSP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG, BAR0_MASK_REG, BAR1_MASK_REG, DBI_DSP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG, DBI_DSP_SEC_STAT_IO_LIMIT_IO_BASE_REG, DBI_DSP_MEM_LIMIT_MEM_BASE REG, DBI_DSP_PREF_MEM_LIMIT_PREF_MEM_BASE_REG, DBI_DSP_CAP_ID_NXT_PTR_REG, DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, DBI_DSP_PL_DEBUG1_OFF",
    "Meta Impacted Registers": "mizar_PCIE0_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG, mizar_PCIE0_DBI_DSP_TYPE1_STATUS_COMMAND_REG, mizar_PCIE0_DBI_DSP_TYPE1_CLASS_CODE_REV_ID_REG, mizar_PCIE0_DBI_DSP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG, mizar_PCIE0_DBI_DSP_BAR0_REG, mizar_PCIE0_DBI_DSP_BAR1_REG, mizar_PCIE0_DBI_DSP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG, mizar_PCIE0_DBI_DSP_SEC_STAT_IO_LIMIT_IO_BASE_REG, mizar_PCIE0_DBI_DSP_MEM_LIMIT_MEM_BASE_REG, mizar_PCIE0_DBI_DSP_PREF_MEM_LIMIT_PREF_MEM_BASE_REG, mizar_PCIE0_DBI_DSP_CAP_ID_NXT_PTR_REG, mizar_PCIE0_DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, mizar_PCIE0_DBI_DSP_PL_DEBUG1_OFF",
    "Validation / Acceptance Criteria": "PASS if: (a) For all readable DBI/DSP registers in the list (excluding DBI_DSP_CAP_ID_NXT_PTR_REG, DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, DBI_DSP_PL_DEBUG1_OFF), the read value matches the documented reset default; (b) For each data pattern applied to writable registers, the readback value equals ((pattern AND read_mask AND write_mask) OR ((NOT write_mask) AND read_mask AND default_value)), confirming writable fields take the pattern and read-only fields retain defaults; and (c) No mismatches are observed across all registers and patterns. FAIL otherwise.",
    "Meta Validation / Acceptance Criteria": "Default check: data_rd == default_value_array[i] for each i where read_mask_array[i] != 0 and i not in the explicit skip set; on mismatch → def_fail_cnt++. Masked write/read: After write, for each i with skip_array[i] != 1, write_mask_array[i] != 0, and read_mask_array[i] != 0, require data_rd == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xffffffff) & read_mask_array[i] & default_value_array[i])); on mismatch → wr_fail_cnt++. Final result: finish(0) only if (def_fail_cnt == 0 && wr_fail_cnt == 0); else finish(1).",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"test_define.c\"\n#include <pcie.h>",
    "Meta Macros": "#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000\n#define CNT 775",
    "Meta Arrays": "const unsigned long int addr_array[20]={mizar_PCIE0_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG,mizar_PCIE0_DBI_DSP_TYPE1_STATUS_COMMAND_REG,mizar_PCIE0_DBI_DSP_TYPE1_CLASS_CODE_REV_ID_REG,mizar_PCIE0_DBI_DSP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG,mizar_PCIE0_DBI_DSP_BAR0_REG,mizar_PCIE0_DBI_DSP_BAR1_REG,mizar_PCIE0_DBI_DSP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG,mizar_PCIE0_DBI_DSP_SEC_STAT_IO_LIMIT_IO_BASE_REG,mizar_PCIE0_DBI_DSP_MEM_LIMIT_MEM_BASE_REG,mizar_PCIE0_DBI_DSP_PREF_MEM_LIMIT_PREF_MEM_BASE_REG,};\n\nconst int default_value_array[20]={PCIE0_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG_DEFAULT_VAL,PCIE0_DBI_DSP_TYPE1_STATUS_COMMAND_REG_DEFAULT_VAL,PCIE0_DBI_DSP_TYPE1_CLASS CODE_REV_ID_REG_DEFAULT_VAL,PCIE0_DBI_DSP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG_DEFAULT_VAL,PCIE0_DBI_DSP_BAR0_REG_DEFAULT_VAL,PCIE0_DBI_DSP_BAR1_REG_DEFAULT_VAL,PCIE0_DBI_DSP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG_DEFAULT_VAL,PCIE0_DBI_DSP_SEC_STAT_IO_LIMIT_IO_BASE_REG_DEFAULT_VAL,PCIE0_DBI_DSP_MEM_LIMIT_MEM_BASE_REG_DEFAULT_VAL,PCIE0_DBI_DSP_PREF_MEM_LIMIT_PREF_MEM_BASE_REG_DEFAULT_VAL,};\n\nconst int read_mask_array[20]={PCIE0_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG_READ_MASK,PCIE0_DBI_DSP_TYPE1_STATUS_COMMAND_REG_READ_MASK,PCIE0_DBI_DSP_TYPE1_CLASS_CODE_REV_ID_REG_READ_MASK,PCIE0_DBI_DSP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG_READ_MASK,PCIE0_DBI_DSP_BAR0_REG_READ_MASK,PCIE0_DBI_DSP_BAR1_REG_READ_MASK,PCIE0_DBI_DSP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG_READ_MASK,PCIE0_DBI_DSP_SEC_STAT_IO_LIMIT_IO_BASE_REG_READ_MASK,PCIE0_DBI_DSP_MEM_LIMIT_MEM_BASE_REG_READ_MASK,PCIE0_DBI_DSP_PREF_MEM_LIMIT_PREF_MEM_BASE_REG_READ_MASK,};\n\nconst int write_mask_array[20]={PCIE0_DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG_WRITE_MASK,PCIE0_DBI_DSP_TYPE1_STATUS_COMMAND_REG_WRITE_MASK,PCIE0_DBI_DSP_TYPE1_CLASS_CODE_REV_ID_REG_WRITE_MASK,PCIE0_DBI_DSP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG_WRITE_MASK,PCIE0_DBI_DSP_BAR0_REG_WRITE_MASK,PCIE0_DBI_DSP_BAR1_REG WRITE_MASK,PCIE0_DBI_DSP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG_WRITE_MASK,PCIE0_DBI_DSP_SEC_STAT_IO_LIMIT_IO_BASE_REG_WRITE_MASK,PCIE0_DBI_DSP_MEM_LIMIT_MEM_BASE_REG_WRITE_MASK,PCIE0_DBI_DSP_PREF_MEM_LIMIT_PREF_MEM_BASE_REG_WRITE_MASK,};\n\nconst int skip_array[20]={0,0,0,0,1,1,0,0,0,1,1,1,0,0,1,1,0,0,0,0,};"
  },
  {
    "Index": "2",
    "SS / Module": "PCIE",
    "Feature": "PF_TYPE0_HDR_DBI2i Registers: Dbi: R/W",
    "Test Case Name": "pcie0_dbi_usp_reg_wr_rd_test",
    "Test Description": "Verify PCIe0 Upstream Port (USP) DBI configuration space registers: check documented reset defaults for readable registers (excluding Capability ID/Next Ptr, Device Control/Status, and PL Debug1), then perform masked write/readback using multiple data patterns to confirm writable bits update while read-only bits retain defaults. Skip any registers marked non-readable or non-writable by masks.",
    "Meta Test Description": "The test defines arrays for addresses, reset defaults, read masks, write masks, and a skip list. Flow: 1) chk_rst_val(): Iterate i=0..CNT-1, addr=addr_array[i]. If read_mask_array[i]==0x00000000, skip. If addr equals DBI_USP_CAP_ID_NXT_PTR_REG or DBI_USP_DEVICE_CONTROL_DEVICE_STATUS or DBI_USP_PL_DEBUG1_OFF, skip default check. Else data_rd=read_reg(addr); compare to default_value_array[i]; on mismatch increment def_fail_cnt and print failure. 2) chk_rd_wr(): For each pattern in {0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}, set data_wr and write to each address if skip_array[i]!=1 and write_mask_array[i]!=0x00000000 using write_reg(addr,data_wr). Then read back for each address if skip_array[i]!=1 and write_mask_array[i]!=0x00000000 and read_mask_array[i]!=0x00000000: data_rd=read_reg(addr); wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if data_rd!=exp_val, increment wr_fail_cnt and log failure. 3) If def_fail_cnt>0 or wr_fail_cnt>0, finish(1); else finish(0). A soft_reset_chk() helper writes SOFT_RST_REG_ADDRESS with SOFT_RST_REG_DATA, waits, then restores default, but it is not invoked.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Default check excludes Capability ID/Next Pointer, Device Control/Status, and PL Debug1 registers. Registers with read mask 0 are treated as not readable; registers with write mask 0 are treated as not writable. Only writable fields are expected to change on writes; read-only fields must remain at reset defaults. The soft reset routine exists but is not executed in this test.",
    "Test Steps / Procedure": "1) Initialize and load the list of PCIe0 USP DBI configuration registers and their masks/defaults. 2) For each readable register (excluding Capability ID/Next Pointer, Device Control/Status, and PL Debug1), read and verify it matches the documented reset default. 3) Apply each test pattern (all-ones, 0xAAAAAAAA, 0x55555555, all-zeros, 0xA5A5A5A5, upper-half ones) to all writable registers not marked to skip. 4) Read back each register that is both writable and readable; verify writable bits reflect the pattern and read-only bits retain their default values per masks. 5) Report PASS only if no mismatches are observed in default checks and masked write/readback across all patterns.",
    "Meta Test Steps / Procedure": "1) Call chk_rst_val(): for (i=0;i<CNT;i++): addr=addr_array[i]; if read_mask_array[i]==0x00000000 → continue; if addr in {mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG, mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF} → continue; data_rd=read_reg(addr); if (data_rd==default_value_array[i]) PASS else {def_fail_cnt++; log}. 2) Call chk_rd_wr(): int chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}; For each j in 0..5: data_wr=chk_val[j]; Write phase: for (i=0;i<CNT;i++): addr=addr_array[i]; if skip_array[i]==1 → continue; if write_mask_array[i]==0x00000000 → continue; else write_reg(addr,data_wr). Read/verify phase: for (i=0;i<CNT;i++): addr=addr_array[i]; if skip_array[i]==1 → continue; if write_mask_array[i]==0x00000000 → continue; if read_mask_array[i]==0x00000000 → continue; data_rd=read_reg(addr); wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd==exp_val) PASS else {wr_fail_cnt++; log}. 3) If (def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0). Note: soft_reset_chk() writes SOFT_RST_REG_ADDRESS, waits, restores previous value, but is not called.",
    "Impacted Registers": "DBI_USP_TYPE1_DEV_ID_VEND_ID_REG, DBI_USP_TYPE1_STATUS_COMMAND_REG, DBI_USP_TYPE1_CLASS_CODE_REV_ID_REG, DBI_USP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE REG, DBI_USP_BAR0_REG, DBI_USP_BAR1_REG, DBI_USP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS REG, DBI_USP_SEC_STAT_IO_LIMIT_IO_BASE REG, DBI_USP_MEM_LIMIT_MEM_BASE REG, DBI_USP_PREF_MEM_LIMIT_PREF_MEM_BASE REG, DBI_USP_PREF_BASE_UPPER_REG, DBI_USP_PREF_LIMIT_UPPER_REG, DBI_USP_IO_LIMIT_UPPER_IO_BASE_UPPER REG, DBI_USP_CAP_ID_NXT_PTR_REG, DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, DBI_USP_PL_DEBUG1_OFF",
    "Meta Impacted Registers": "mizar_PCIE0_DBI_USP_TYPE1_DEV_ID_VEND_ID_REG, mizar_PCIE0_DBI_USP_TYPE1_STATUS_COMMAND_REG, mizar_PCIE0_DBI_USP_TYPE1_CLASS_CODE_REV_ID_REG, mizar_PCIE0_DBI_USP_TYPE1_BIST HDR_TYPE_LAT_CACHE_LINE_SIZE_REG, mizar_PCIE0_DBI_USP_BAR0_REG, mizar_PCIE0_DBI_USP_BAR1_REG, mizar_PCIE0_DBI_USP_SEC LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG, mizar_PCIE0_DBI_USP_SEC_STAT_IO_LIMIT_IO_BASE_REG, mizar_PCIE0_DBI_USP_MEM_LIMIT_MEM_BASE_REG, mizar_PCIE0_DBI_USP_PREF_MEM_LIMIT_PREF_MEM_BASE_REG, mizar_PCIE0_DBI_USP_PREF_BASE_UPPER_REG, mizar_PCIE0_DBI_USP_PREF_LIMIT_UPPER_REG, mizar_PCIE0_DBI_USP_IO_LIMIT_UPPER_IO_BASE_UPPER_REG, mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG, mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF",
    "Validation / Acceptance Criteria": "PASS if: 1) All readable DBI USP configuration registers (excluding Capability ID/Next Pointer, Device Control/Status, and PL Debug1) match their reset defaults; 2) For each applied pattern, readback equals (pattern AND read_mask AND write_mask) OR ((NOT write_mask) AND read_mask AND default_value) for every register that is both writable and readable; 3) No mismatches are reported across all checks and patterns.",
    "Meta Validation / Acceptance Criteria": "Default check: require data_rd == default_value_array[i] for each i with read_mask_array[i] != 0 and addr not in {mizar_PCIE0_DBI_USP_CAP_ID_NXT_PTR_REG, mizar_PCIE0_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, mizar_PCIE0_DBI_USP_PL_DEBUG1_OFF}; on mismatch → def_fail_cnt++. Masked R/W: for each tested i with skip_array[i] != 1, write_mask_array[i] != 0, read_mask_array[i] != 0, require data_rd == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xffffffff) & read_mask_array[i] & default_value_array[i])); on mismatch → wr_fail_cnt++. Final: finish(0) if (def_fail_cnt == 0 && wr_fail_cnt == 0), else finish(1).",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"test_define.c\"\n#include <pcie.h>",
    "Meta Macros": "#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG DATA 0x00000000\n#define CNT 775",
    "Meta Arrays": "const unsigned long int addr_array[20]={mizar_PCIE0_DBI_USP_TYPE1_DEV_ID_VEND_ID_REG,mizar_PCIE0_DBI_USP_TYPE1_STATUS_COMMAND_REG,mizar_PCIE0_DBI_USP_TYPE1_CLASS_CODE_REV_ID_REG,mizar_PCIE0_DBI_USP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG,mizar_PCIE0_DBI_USP_BAR0_REG,mizar_PCIE0_DBI_USP_BAR1_REG,mizar_PCIE0_DBI_USP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG,mizar_PCIE0_DBI_USP_SEC_STAT_IO_LIMIT_IO_BASE_REG,mizar_PCIE0_DBI_USP_MEM_LIMIT_MEM_BASE_REG,mizar_PCIE0_DBI_USP_PREF_MEM_LIMIT_PREF_MEM_BASE_REG,mizar_PCIE0_DBI_USP_PREF_BASE_UPPER_REG,mizar_PCIE0_DBI_USP_PREF_LIMIT_UPPER REG,mizar_PCIE0_DBI_USP_IO_LIMIT_UPPER_IO_BASE_UPPER_REG,};\n\nconst int default_value_array[20]={PCIE0_DBI_USP_TYPE1_DEV_ID_VEND_ID_REG_DEFAULT_VAL,PCIE0_DBI_USP_TYPE1_STATUS_COMMAND_REG_DEFAULT_VAL,PCIE0_DBI_USP_TYPE1_CLASS_CODE_REV_ID_REG_DEFAULT_VAL,PCIE0_DBI_USP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG_DEFAULT_VAL,PCIE0_DBI_USP_BAR0_REG_DEFAULT_VAL,PCIE0_DBI_USP_BAR1_REG_DEFAULT_VAL,PCIE0_DBI_USP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG_DEFAULT_VAL,PCIE0_DBI_USP_SEC_STAT_IO_LIMIT_IO_BASE REG_DEFAULT_VAL,PCIE0_DBI_USP_MEM_LIMIT_MEM_BASE_REG_DEFAULT_VAL,PCIE0_DBI_USP_PREF_MEM_LIMIT_PREF_MEM_BASE_REG_DEFAULT_VAL,};\n\nconst int read_mask_array[20]={PCIE0_DBI_USP_TYPE1_DEV_ID_VEND_ID_REG_READ_MASK,PCIE0_DBI_USP_TYPE1_STATUS_COMMAND_REG_READ_MASK,PCIE0_DBI_USP_TYPE1_CLASS_CODE_REV_ID_REG_READ_MASK,PCIE0_DBI_USP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE REG_READ_MASK,PCIE0_DBI_USP_BAR0_REG_READ_MASK,PCIE0_DBI_USP_BAR1_REG_READ_MASK,PCIE0_DBI_USP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG_READ_MASK,PCIE0_DBI_USP_SEC_STAT_IO_LIMIT_IO_BASE_REG_READ_MASK,PCIE0_DBI_USP_MEM_LIMIT_MEM_BASE_REG_READ_MASK,PCIE0_DBI_USP_PREF_MEM_LIMIT_PREF_MEM_BASE REG_READ_MASK,};\n\nconst int write_mask_array[20]={PCIE0_DBI_USP_TYPE1_DEV_ID_VEND_ID_REG_WRITE_MASK,PCIE0_DBI_USP_TYPE1_STATUS_COMMAND_REG_WRITE_MASK,PCIE0_DBI_USP_TYPE1_CLASS_CODE_REV_ID_REG_WRITE_MASK,PCIE0_DBI_USP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE REG_WRITE_MASK,PCIE0_DBI_USP_BAR0_REG_WRITE_MASK,PCIE0_DBI_USP_BAR1_REG_WRITE_MASK,PCIE0_DBI_USP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG_WRITE_MASK,PCIE0_DBI_USP_SEC_STAT_IO_LIMIT_IO_BASE_REG_WRITE_MASK,PCIE0_DBI_USP_MEM_LIMIT_MEM_BASE REG_WRITE_MASK,PCIE0_DBI_USP_PREF_MEM_LIMIT_PREF_MEM_BASE REG_WRITE_MASK,};\n\nconst int skip_array[20]={0,0,0,0,1,1,0,0,0,1,1,1,0,0,1,1,0,0,0,0,};"
  },
  {
    "Index": "3",
    "SS / Module": "PCIE",
    "Feature": "Testable: writeAsRead",
    "Test Case Name": "pcie0_sii_rc_reg_wr_rd_test",
    "Test Description": "Validate PCIe0 SII Root Complex configuration window registers by: (1) checking default reset values for all readable registers (excluding SII_PHY_RST_CONTROL), and (2) performing masked write/readback using multiple data patterns so that writable bits reflect the pattern while read‑only bits retain their defaults. Registers flagged as non‑readable or non‑writable by masks are skipped. Test passes if no mismatches are reported.",
    "Meta Test Description": "The test runs two phases. Phase 1 (Default Check): Iterate i=0..CNT-1 over an address list. If read_mask_array[i]==0x00000000, skip. If the address equals SII_PHY_RST_CONTROL, skip default check. Otherwise read_reg(addr_array[i]) and compare with default_value_array[i]; on mismatch increment def_fail_cnt and print failure. Phase 2 (Masked R/W): For each pattern in {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000}, write the pattern to each address if skip_array[i]!=1 and write_mask_array[i]!=0x00000000 using write_reg(addr,data_wr). Then for each address with skip_array[i]!=1, write_mask_array[i]!=0x00000000, and read_mask_array[i]!=0x00000000, read_reg(addr) and compute exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i]^0xffffffff) & read_mask_array[i] & default_value_array[i])). If data_rd!=exp_val, increment wr_fail_cnt and log failure. At the end, if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1) else finish(0). A soft_reset_chk() helper writes SOFT_RST_REG_ADDRESS, waits, and restores the previous value, but it is not executed.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "0xE68C0000",
    "Memory End Offset": "NA",
    "Remarks": "Default-value verification excludes SII_PHY_RST_CONTROL to avoid side effects. Addresses with read mask 0 are treated as not readable and are skipped. Addresses with write mask 0 are treated as not writable and are skipped during writes and readback verification. The soft reset routine is available but not invoked during this test.",
    "Test Steps / Procedure": "1) Initialize and load the PCIe0 SII RC configuration register list, including associated reset defaults and read/write masks. 2) Verify default values for each readable register; skip SII_PHY_RST_CONTROL and any register marked not readable. 3) For each test pattern (all-ones, 0xAAAAAAAA, 0x55555555, all-zeros, 0xA5A5A5A5, upper-half ones), write the pattern to every register flagged writable and not skipped. 4) Read back each register that is both writable and readable; confirm writable bits reflect the pattern and read-only bits retain the documented default. 5) Aggregate all mismatches from default checks and masked write/readback; declare PASS only if no mismatches are detected.",
    "Meta Test Steps / Procedure": "1) Call chk_rst_val(): for (i=0;i<CNT;i++): addr=addr_array[i]; if (read_mask_array[i]==0x00000000) continue; if (addr_array[i]==mizar_PCIE0_SII_PHY_RST_CONTROL) continue; data_rd=read_reg(addr); if (data_rd==default_value_array[i]) PASS log else {def_fail_cnt++; FAIL log}. 2) Call chk_rd_wr(): int chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}; For each j in 0..5: data_wr=chk_val[j]; Write phase: for (i=0;i+CNT;i++): addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0x00000000) continue; write_reg(addr,data_wr). Read/verify phase: for (i=0;i<CNT;i++): addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0x00000000) continue; if (read_mask_array[i]==0x00000000) continue; data_rd=read_reg(addr); wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd==exp_val) PASS log else {wr_fail_cnt++; FAIL log}. 3) If (def_fail_cnt>0 || wr_fail_cnt>0) finish(1); else finish(0). Note: soft_reset_chk() writes SOFT_RST_REG_ADDRESS, waits, restores original value; not invoked.",
    "Impacted Registers": "SII_CFG_BAR0_START1, SII_CFG_BAR0_START2, SII_CFG_BAR0_LIMIT1, SII_CFG_BAR0_LIMIT2, SII_CFG_BAR1_START, SII_CFG_BAR1_LIMIT1, SII_CFG_BAR2_START1, SII_CFG_BAR2_START2, SII_CFG_BAR2_LIMIT1, SII_CFG_BAR2_LIMIT2, SII_PHY_RST_CONTROL",
    "Meta Impacted Registers": "mizar_PCIE0_SII_CFG_BAR0_START1, mizar_PCIE0_SII_CFG_BAR0_START2, mizar_PCIE0_SII_CFG_BAR0_LIMIT1, mizar_PCIE0_SII_CFG_BAR0_LIMIT2, mizar_PCIE0_SII_CFG_BAR1_START, mizar_PCIE0_SII_CFG_BAR1_LIMIT1, mizar_PCIE0_SII_CFG_BAR2_START1, mizar_PCIE0_SII_CFG_BAR2_START2, mizar_PCIE0_SII_CFG_BAR2_LIMIT1, mizar_PCIE0_SII_CFG_BAR2_LIMIT2, mizar_PCIE0_SII_PHY_RST_CONTROL",
    "Validation / Acceptance Criteria": "PASS if: (a) For all readable SII RC registers (excluding SII_PHY_RST_CONTROL), the read value equals the documented reset default; (b) For each applied pattern, each register that is both writable and readable returns (pattern AND read_mask AND write_mask) OR ((NOT write_mask) AND read mask AND default value); and (c) No mismatches are logged across all registers and patterns. Otherwise, FAIL.",
    "Meta Validation / Acceptance Criteria": "Default check requires data_rd == default_value_array[i] for each i with read_mask_array[i] != 0 and address != mizar_PCIE0_SII_PHY_RST_CONTROL; on mismatch → def_fail_cnt++. Masked R/W requires for each i with skip_array[i] != 1, write_mask_array[i] != 0, read_mask_array[i] != 0: data_rd == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i]^0xffffffff) & read_mask_array[i] & default_value_array[i])); on mismatch → wr_fail_cnt++. Final result: finish(0) only if (def_fail_cnt == 0 && wr_fail_cnt == 0); else finish(1).",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"test_define.c\"\n#include <pcie0/pcie_sii_rc_def.h>\n#include <pcie0/pcie_sii_rc_offset.h>",
    "Meta Macros": "#define MIZAR_PCIE0_SII_BASE 0xE68C0000\n#define CNT 153\n#define SOFT_RST_REG_ADDRESS 0x00000000\n#define SOFT_RST_REG_DATA 0x00000000",
    "Meta Arrays": "const unsigned long int addr_array[20]={mizar_PCIE0_SII_CFG_BAR0_START1,mizar_PCIE0_SII_CFG_BAR0_START2,mizar_PCIE0_SII_CFG_BAR0_LIMIT1,mizar_PCIE0_SII_CFG_BAR0_LIMIT2,mizar_PCIE0_SII_CFG_BAR1_START,mizar_PCIE0_SII_CFG_BAR1_LIMIT1,mizar_PCIE0_SII_CFG_BAR2_START1,mizar_PCIE0_SII_CFG_BAR2_START2,mizar_PCIE0_SII_CFG_BAR2_LIMIT1,mizar_PCIE0_SII_CFG_BAR2_LIMIT2,};\n\nconst int default_value_array[20]={PCIE0_SII_CFG_BAR0_START1_DEFAULT_VAL,PCIE0_SII_CFG_BAR0_START2_DEFAULT_VAL,PCIE0_SII_CFG_BAR0_LIMIT1_DEFAULT_VAL,PCIE0_SII_CFG_BAR0_LIMIT2_DEFAULT_VAL,PCIE0_SII_CFG_BAR1_START_DEFAULT_VAL,PCIE0_SII_CFG_BAR1_LIMIT1_DEFAULT_VAL,PCIE0_SII_CFG_BAR2_START1_DEFAULT_VAL,PCIE0_SII_CFG_BAR2_START2_DEFAULT_VAL,PCIE0_SII_CFG_BAR2_LIMIT1_DEFAULT_VAL,PCIE0_SII_CFG_BAR2_LIMIT2_DEFAULT_VAL,};\n\nconst int read_mask_array[20]={PCIE0_SII_CFG_BAR0_START1_READ_MASK,PCIE0_SII_CFG_BAR0_START2_READ_MASK,PCIE0_SII_CFG_BAR0_LIMIT1_READ_MASK,PCIE0_SII_CFG_BAR0_LIMIT2_READ_MASK,PCIE0_SII_CFG_BAR1_START_READ_MASK,PCIE0_SII_CFG_BAR1_LIMIT1_READ_MASK,PCIE0_SII_CFG_BAR2_START1_READ_MASK,PCIE0_SII_CFG_BAR2_START2_READ_MASK,PCIE0_SII_CFG_BAR2_LIMIT1_READ_MASK,PCIE0_SII_CFG_BAR2_LIMIT2_READ_MASK,};\n\nconst int write_mask_array[20]={PCIE0_SII_CFG_BAR0_START1_WRITE_MASK,PCIE0_SII_CFG_BAR0_START2_WRITE_MASK,PCIE0_SII_CFG_BAR0_LIMIT1_WRITE_MASK,PCIE0_SII_CFG_BAR0_LIMIT2_WRITE_MASK,PCIE0_SII_CFG_BAR1_STARTWRITE_MASK,PCIE0_SII_CFG_BAR1_LIMIT1_WRITE_MASK,PCIE0_SII_CFG_BAR2_START1_WRITE_MASK,PCIE0_SII_CFG_BAR2_START2_WRITE_MASK,PCIE0_SII_CFG_BAR2_LIMIT1_WRITE_MASK,PCIE0_SII_CFG_BAR2_LIMIT2_WRITE_MASK,};\n\nconst int skip_array[20]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,};"
  },
  {
    "Index": "4",
    "SS / Module": "PCIE",
    "Feature": "Testable: writeAsRead",
    "Test Case Name": "pcie1_dbi_dsp_reg_wr_rd_test",
    "Test Description": "Validate PCIe1 DBI/DSP configuration registers by checking documented reset defaults for readable registers, then performing masked write/readback with data patterns to confirm writable bits update while read-only bits retain their defaults. Registers that are not readable or writable are skipped. Test passes if no mismatches are observed.",
    "Meta Test Description": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Skip any configuration registers that are not readable or writable as per their access attributes. Ensure capability and status/control registers that can change due to side effects are handled carefully during default checks.",
    "Test Steps / Procedure": "1) Identify the PCIe1 DBI/DSP configuration registers in scope for validation. 2) Read all registers that are specified as readable and verify their values match the documented reset defaults. 3) For each writable register, apply a set of data patterns and write using the register's write mask. 4) Read back each written register and verify only the writable fields change while read-only fields retain their default values. 5) Record any mismatches and declare the test PASS only if no discrepancies are found across all checks and patterns.",
    "Meta Test Steps / Procedure": "NA",
    "Impacted Registers": "DBI_DSP_TYPE1_DEV_ID_VEND_ID_REG, DBI_DSP_TYPE1_STATUS_COMMAND REG, DBI_DSP_TYPE1_CLASS_CODE_REV_ID_REG, DBI_DSP_TYPE1_BIST_HDR_TYPE_LAT CACHE_LINE_SIZE_REG, DBI_DSP_BAR0_REG, DBI_DSP_BAR1_REG, DBI_DSP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG, DBI_DSP_SEC_STAT_IO_LIMIT_IO_BASE REG, DBI_DSP_MEM_LIMIT_MEM_BASE REG, DBI_DSP_PREF_MEM_LIMIT_PREF_MEM_BASE REG, DBI_DSP_CAP_ID_NXT_PTR_REG, DBI_DSP_DEVICE_CONTROL_DEVICE_STATUS, DBI_DSP_PL_DEBUG1_OFF",
    "Meta Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "PASS if: 1) Every readable DBI/DSP register matches its documented reset default; 2) After applying patterns to writable registers, readback equals (pattern AND read_mask AND write_mask) OR ((NOT write_mask) AND read_mask AND default_value), proving writable fields update while read-only fields remain at defaults; 3) No mismatches are reported across all registers and patterns. Otherwise, FAIL.",
    "Meta Validation / Acceptance Criteria": "NA",
    "Code Generation (Required / Not)": "NA",
    "Meta Headers": "NA",
    "Meta Macros": "NA",
    "Meta Arrays": "NA"
  },
  {
    "Index": "5",
    "SS / Module": "PCIE",
    "Feature": "Upstream Port Value After Reset and Testable: writeAsRead",
    "Test Case Name": "pcie1_dbi_usp_reg_wr_rd_test",
    "Test Description": "Validate PCIe1 Upstream Port (USP) DBI configuration space registers by first checking reset defaults for all readable registers, then performing masked write/readback with multiple data patterns to confirm writable bits update while read-only bits retain defaults. Capability pointer, device control/status, and platform debug registers are handled carefully or excluded from default checks to avoid side effects.",
    "Meta Test Description": "The test defines parallel arrays: addr_array[] (register addresses), default_value_array[] (documented reset defaults), read_mask_array[] (readable bits), write_mask_array[] (writable bits), and skip_array[] (registers to skip due to side effects or access restrictions). Flow: 1) Default-value verification (chk_rst_val): Iterate i over all entries. If read_mask_array[i] == 0x00000000, skip (not readable). If addr is one of {mizar_PCIE1_DBI_USP_CAP_ID_NXT_PTR_REG, mizar_PCIE1_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, mizar_PCIE1_DBI_USP_PL_DEBUG1_OFF}, skip default check. Otherwise, data_rd = read_reg(addr_array[i]); compare against default_value_array[i]; on mismatch, increment def_fail_cnt and log failure. 2) Masked write/read verification (chk_rd_wr): For each data pattern in {0xffffffff, 0xaaaaaaaa, 0x55555555, 0x00000000, 0xA5A5A5A5, 0xffff0000}: (a) Write phase: for each i, if skip_array[i] == 1, continue; if write_mask_array[i] == 0x00000000, continue; otherwise write_reg(addr_array[i], data_wr). (b) Read/verify phase: for each i, if skip_array[i] == 1, continue; if write_mask_array[i] == 0x00000000, continue; if read_mask_array[i] == 0x00000000, continue; then data_rd = read_reg(addr_array[i]); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd != exp_val) increment wr_fail_cnt and log failure. 3) Finalize: if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1) else finish(0). A soft_reset_chk() helper may exist (write a soft reset register, wait, restore) but is not used in the normal flow.",
    "Speed": "NA",
    "Mode": "Polling",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "Exclude the Capability ID/Next Pointer, Device Control/Status, and PL Debug1 registers from reset-default checks to avoid dynamic changes during reads. Treat registers with no readable bits as not readable and skip them. Treat registers with no writable bits as not writable and skip pattern writes. Validate that only writable fields change; read-only fields must remain at reset defaults.",
    "Test Steps / Procedure": "1) Load the list of PCIe1 USP DBI configuration registers with their reset defaults and read/write masks. 2) For each readable register (excluding capability pointer, device control/status, and PL Debug1), read and verify it matches the documented reset default. 3) Apply multiple data patterns (all-ones, 0xAAAAAAAA, 0x55555555, all-zeros, 0xA5A5A5A5, upper-half ones) to all registers that are designated writable and not skipped. 4) Read back each register that is both writable and readable; verify writable bits reflect the pattern while read-only bits retain their default values. 5) Aggregate results across all registers and patterns; declare PASS only if no mismatches are found.",
    "Meta Test Steps / Procedure": "1) chk_rst_val(): for (i=0; i<CNT; i++) { addr=addr_array[i]; if (read_mask_array[i]==0x00000000) continue; if (addr==mizar_PCIE1_DBI_USP_CAP_ID_NXT_PTR_REG || addr==mizar_PCIE1_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS || addr==mizar_PCIE1_DBI_USP_PL_DEBUG1_OFF) continue; data_rd=read_reg(addr); if (data_rd==default_value_array[i]) pass_log(); else {def_fail_cnt++; fail_log(i, addr, data_rd, default_value_array[i]);} } 2) chk_rd_wr(): int patterns[6]={0xffffffff,0xaaaaaaaa,0x55555555,0x00000000,0xA5A5A5A5,0xffff0000}; for each pattern data_wr in patterns { # write phase for (i=0; i<CNT; i++) { addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0x00000000) continue; write_reg(addr, data_wr); } # read/verify phase for (i=0; i<CNT; i++) { addr=addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0x00000000) continue; if (read_mask_array[i]==0x00000000) continue; data_rd=read_reg(addr); wr_n=(write_mask_array[i]^0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd==exp_val) pass_log(); else {wr_fail_cnt++; fail_log(i, addr, data_rd, exp_val);} } } 3) if (def_fail_cnt>0 || wr_fail_cnt>0) finish(1); else finish(0).",
    "Impacted Registers": "DBI_USP_TYPE1_DEV_ID_VEND_ID_REG, DBI_USP_TYPE1_STATUS_COMMAND_REG, DBI_USP_TYPE1_CLASS_CODE_REV_ID_REG, DBI_USP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG, DBI_USP_BAR0_REG, DBI_USP_BAR1 REG, DBI_USP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS_REG, DBI_USP_SEC_STAT_IO_LIMIT_IO_BASE_REG, DBI_USP_MEM_LIMIT_MEM_BASE_REG, DBI_USP_PREF_MEM_LIMIT_PREF_MEM_BASE REG, DBI_USP_PREF_BASE_UPPER_REG, DBI_USP_PREF_LIMIT_UPPER_REG, DBI_USP_IO_LIMIT_UPPER_IO_BASE_UPPER_REG, DBI_USP_CAP_ID_NXT_PTR_REG, DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, DBI_USP_PL_DEBUG1_OFF",
    "Meta Impacted Registers": "mizar_PCIE1_DBI_USP_TYPE1_DEV_ID_VEND_ID_REG, mizar_PCIE1_DBI_USP_TYPE1_STATUS_COMMAND_REG, mizar_PCIE1_DBI_USP_TYPE1_CLASS_CODE_REV_ID_REG, mizar_PCIE1_DBI_USP_TYPE1_BIST_HDR_TYPE_LAT_CACHE_LINE_SIZE_REG, mizar_PCIE1_DBI_USP_BAR0_REG, mizar_PCIE1_DBI_USP_BAR1_REG, mizar_PCIE1_DBI_USP_SEC_LAT_TIMER_SUB_BUS_SEC_BUS_PRI_BUS REG, mizar_PCIE1_DBI_USP_SEC_STAT_IO_LIMIT_IO_BASE_REG, mizar_PCIE1_DBI_USP_MEM_LIMIT_MEM_BASE_REG, mizar_PCIE1_DBI_USP_PREF_MEM_LIMIT_PREF_MEM_BASE REG, mizar_PCIE1_DBI_USP_PREF_BASE_UPPER_REG, mizar_PCIE1_DBI_USP_PREF_LIMIT_UPPER REG, mizar_PCIE1_DBI_USP_IO_LIMIT_UPPER_IO_BASE_UPPER_REG, mizar_PCIE1_DBI_USP_CAP_ID_NXT_PTR_REG, mizar_PCIE1_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, mizar_PCIE1_DBI_USP_PL_DEBUG1_OFF",
    "Validation / Acceptance Criteria": "PASS if: 1) All readable DBI USP configuration registers (excluding Capability ID/Next Pointer, Device Control/Status, and PL Debug1) match their reset defaults; 2) For each applied pattern, each register that is both writable and readable returns (pattern AND readable_bits AND writable_bits) OR ((NOT writable_bits) AND readable_bits AND reset_default); 3) No mismatches are reported across all checks and patterns. Otherwise, FAIL.",
    "Meta Validation / Acceptance Criteria": "Default check: require data_rd == default_value_array[i] for each i with (read_mask_array[i] != 0) and addr not in {mizar_PCIE1_DBI_USP_CAP_ID_NXT_PTR_REG, mizar_PCIE1_DBI_USP_DEVICE_CONTROL_DEVICE_STATUS, mizar_PCIE1_DBI_USP_PL_DEBUG1_OFF}; on mismatch → def_fail_cnt++. Masked R/W: for each tested i with skip_array[i] != 1, write_mask_array[i] != 0, read_mask_array[i] != 0, require data_rd == ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((write_mask_array[i] ^ 0xffffffff) & read_mask_array[i] & default_value_array[i])); on mismatch → wr_fail_cnt++. Final: finish(0) if (def_fail_cnt == 0 && wr_fail_cnt == 0), else finish(1).",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "#include <stdio.h>\n#include <stdlib.h>\n#include \"test_common.h\"\n#include \"test_define.c\"\n#include <pcie.h>",
    "Meta Macros": "NA",
    "Meta Arrays": "NA"
  },
  {
    "Index": "6",
    "SS / Module": "PCIE",
    "Feature": "Testable: writeAsRead",
    "Test Case Name": "pcie1_sii_rc_reg_wr_rd_test",
    "Test Description": "Verify SII Root Complex configuration window registers for write-as-read behavior: confirm documented reset values where readable and validate that only writable bits update across test patterns while read-only bits remain unchanged.",
    "Meta Test Description": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "NA",
    "Test Steps / Procedure": "1) Enumerate the SII Root Complex configuration registers in the PCIe1 instance. 2) Read all registers that are marked readable and record their current values as reset defaults. 3) For each writable register, apply a set of data patterns and perform masked writes. 4) Read back the registers and verify that writable fields reflect the written pattern while read-only fields remain at documented reset values. 5) Log any mismatches and declare PASS only if no discrepancies are found across all checks and patterns.",
    "Meta Test Steps / Procedure": "NA",
    "Impacted Registers": "SII_CFG_BAR0_START1, SII_CFG_BAR0_START2, SII_CFG_BAR0_LIMIT1, SII_CFG_BAR0_LIMIT2, SII_CFG_BAR1_START, SII_CFG_BAR1_LIMIT1, SII_CFG_BAR2_START1, SII_CFG_BAR2_START2, SII_CFG_BAR2_LIMIT1, SII_CFG_BAR2_LIMIT2, SII_PHY_RST_CONTROL",
    "Meta Impacted Registers": "mizar_PCIE1_SII_CFG_BAR0_START1, mizar_PCIE1_SII_CFG_BAR0_START2, mizar_PCIE1_SII_CFG_BAR0_LIMIT1, mizar_PCIE1_SII_CFG_BAR0_LIMIT2, mizar_PCIE1_SII_CFG_BAR1_START, mizar_PCIE1_SII_CFG_BAR1_LIMIT1, mizar_PCIE1_SII_CFG_BAR2_START1, mizar_PCIE1_SII_CFG_BAR2_START2, mizar_PCIE1_SII_CFG_BAR2_LIMIT1, mizar_PCIE1_SII_CFG_BAR2_LIMIT2, mizar_PCIE1_SII_PHY_RST_CONTROL",
    "Validation / Acceptance Criteria": "PASS if: 1) All readable SII RC registers match their documented reset defaults; 2) After applying patterns to writable registers, readback equals (pattern AND readable_bits AND writable_bits) OR ((NOT writable_bits) AND readable_bits AND reset_default), proving writable fields update while read-only fields remain at defaults; 3) No mismatches are reported across all registers and patterns. Otherwise, FAIL.",
    "Meta Validation / Acceptance Criteria": "NA",
    "Code Generation (Required / Not)": "Not",
    "Meta Headers": "NA",
    "Meta Macros": "NA",
    "Meta Arrays": "NA"
  },
  {
    "Index": "7",
    "SS / Module": "PCIE",
    "Feature": "Testable: writeAsRead",
    "Test Case Name": "pcie_reg_wr_rd_test",
    "Test Description": "Perform register read/write verification on PCIe configuration/register space to ensure writable bits update correctly while read-only bits retain their reset defaults across multiple data patterns.",
    "Meta Test Description": "NA",
    "Speed": "NA",
    "Mode": "NA",
    "Memory Start Offset": "NA",
    "Memory End Offset": "NA",
    "Remarks": "NA",
    "Test Steps / Procedure": "NA",
    "Meta Test Steps / Procedure": "NA",
    "Impacted Registers": "NA",
    "Meta Impacted Registers": "NA",
    "Validation / Acceptance Criteria": "PASS if, for all targeted PCIe registers, readback after each applied data pattern matches the expected masked value for writable fields and preserves documented defaults for read-only fields, with no mismatches reported. FAIL otherwise.",
    "Meta Validation / Acceptance Criteria": "NA",
    "Code Generation (Required / Not)": "NA",
    "Meta Headers": "NA",
    "Meta Macros": "NA",
    "Meta Arrays": "NA"
  }
]
''')

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
    "Code Generation (Required / Not)",
]

METADATA_COLUMNS = [
    "Index",
    "Test Case Name",
    "Meta Test Description",
    "Meta Test Steps / Procedure",
    "Meta Impacted Registers",
    "Meta Validation / Acceptance Criteria",
    "Meta Headers",
    "Meta Macros",
    "Meta Arrays",
]


def value_of(obj: Dict[str, Any], key: str) -> Any:
    v = obj.get(key, "")
    if v is None:
        return ""
    return v


def build_workbook(data: List[Dict[str, Any]]) -> Workbook:
    wb = Workbook()
    # Remove the default sheet
    default_sheet = wb.active
    wb.remove(default_sheet)

    ws_plan = wb.create_sheet(title="TestPlan")
    ws_meta = wb.create_sheet(title="MetaData")

    # Write headers with bold font and freeze pane
    bold = Font(bold=True)

    for col_idx, h in enumerate(TESTPLAN_COLUMNS, start=1):
        c = ws_plan.cell(row=1, column=col_idx, value=h)
        c.font = bold
    ws_plan.freeze_panes = "A2"

    for col_idx, h in enumerate(METADATA_COLUMNS, start=1):
        c = ws_meta.cell(row=1, column=col_idx, value=h)
        c.font = bold
    ws_meta.freeze_panes = "A2"

    # Populate rows
    for r_idx, item in enumerate(data, start=2):
        # TestPlan sheet
        for c_idx, key in enumerate(TESTPLAN_COLUMNS, start=1):
            ws_plan.cell(row=r_idx, column=c_idx, value=value_of(item, key))
        # MetaData sheet
        for c_idx, key in enumerate(METADATA_COLUMNS, start=1):
            ws_meta.cell(row=r_idx, column=c_idx, value=value_of(item, key))

    # Set MetaData sheet to very hidden
    ws_meta.sheet_state = 'veryHidden'

    return wb


def ist_now_stamp() -> str:
    # IST = UTC+5:30
    ist = timezone(timedelta(hours=5, minutes=30))
    return datetime.now(ist).strftime('%Y%m%d_%H%M%S')


def main() -> None:
    # Validate JSON
    if not isinstance(JSON_DATA, list):
        raise SystemExit("json_data is not a list")

    # Build workbook
    wb = build_workbook(JSON_DATA)

    # Prepare output path
    out_dir = os.path.join("Test_Output", "PCIE", "TestPlan")
    os.makedirs(out_dir, exist_ok=True)
    filename = f"testplan_{ist_now_stamp()}.xlsx"
    out_path = os.path.join(out_dir, filename)

    # Save as a real .xlsx file
    wb.save(out_path)
    print(f"Generated: {out_path}")


if __name__ == '__main__':
    main()
