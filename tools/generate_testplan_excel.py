import json, os
from datetime import datetime, timezone, timedelta
from openpyxl import Workbook
from openpyxl.styles import Font, Alignment, PatternFill, Border, Side
from openpyxl.worksheet.datavalidation import DataValidation
from openpyxl.utils import get_column_letter

# Embedded JSON input (Stage1-compatible)
JSON_DATA = r'''{
  "TestCases": [
    {
      "Index": "1",
      "SS / Module": "GPIO",
      "Feature": "Independent control register for each GPIO",
      "Test Case Name": "gpio_reg_wr_rd_test",
      "Test Description": "Verify per-pin and group GPIO registers for correct reset values and read/write behavior using defined read/write masks across the GPIO_GP0_GPIO_8..GPIO_GP0_GPIO_39 and group control/status registers.",
      "Speed": "NA",
      "Mode": "NA",
      "Memory Start Offset": "NA",
      "Memory End Offset": "NA",
      "Remarks": "When reading default values, input data may float high without forced driving; forcing input low can alter selection and cause mismatches. Skip lists are applied for non-R/W registers during checks.",
      "Test Steps / Procedure": "Entry is through the test case. 1) Check reset values: iterate over 49 registers (GPIO_GP0_GPIO_8..GPIO_GP0_GPIO_39, GPIO_GPIO_INTR_RAW_STCLR1, GPIO_GP0_INTR1_INTR_EN1, GPIO_GP0_INTR1_INTR_STS1, GPIO_GP0_INTR2_INTR_EN1, GPIO_GP0_INTR2_INTR_STS1, GPIO_GPIO_IO_CTRL_GROUP1..4, GPIO_GPIO_DOUT_GROUP1..4, GPIO_GPIO_DIN_GROUP1..4). For each index: a) If the corresponding skip-for-reset flag is set, skip this register. b) If the corresponding read mask is zero, skip reading for this register. c) Read the register. d) Mask the read value to ignore bit 0. e) Compare masked read value with the expected default value for that index; on mismatch, record a default-value failure. 2) Write and readback checks: For each of six test patterns (0xffffffff, 0xaaaaaaaa, 0x55555555, 0xf5f5f5f5, 0xA5A5A5A5, 0xffff0000): a) For each of the 49 registers, if the register is marked to be skipped or has a zero write mask, skip writing; otherwise write the pattern masked by that register’s write mask. b) For each of the 49 registers, if skipped or not writable or not readable per masks, skip reading; otherwise read the register and mask by its read mask. c) Form the expected value as (pattern & read_mask & write_mask) OR (~write_mask & read_mask & default_value). d) Compare the masked readback value with the expected value; on mismatch, record a write/read failure. 3) Finalize result: if any default-value or write/read failures were recorded, return failure; otherwise return pass.",
      "Impacted Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
      "Validation / Acceptance Criteria": "Pass if and only if: 1) For every checked register, the masked reset value equals its corresponding expected default value. 2) For every checked register and for each test pattern, the masked readback equals the expected value derived from write mask, read mask, and default value. Otherwise, fail.",
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
      "Hidden_Test_Description": "Register default and masked read/write verification over GPIO address list using arrays: addr_array, default_value_array, read_mask_array, write_mask_array; skips per skip_array and skip_rst_array.",
      "Hidden_Remarks": "when reading default values the din value is becoming 1 automatically if we don't force any value,but if we force zero to din bit level sel becoming high,so that reding value not matched with expected value",
      "Hidden_Test_Steps_Procedure": "Entry: int test_case() in program.c. 1) chk_rst_val(): for(i=0;i<CNT;i++): addr=addr_array[i]; if(skip_rst_array[i]==1) continue; if(read_mask_array[i]==0x00000000) continue; data_rd=read_reg(addr); data=(data_rd & 0xfffffffe); if(data==default_value_array[i]) pass; else {def_fail_cnt++; printf(\"RST : Failed Default value mismatch Addr :0x%x Expected : 0x%x\\tRead_data : 0x%x\\tDATA : 0x%x\\n\",addr,default_value_array[i],data,data_rd);} 2) chk_rd_wr(): unsigned int chk_val[6]={0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}; for each j: data_wr=chk_val[j]; // write phase: for(i=0;i<CNT;i++){ addr=addr_array[i]; if(skip_array[i]==1) continue; if(write_mask_array[i]==0x00000000) continue; write_reg(addr,(data_wr & write_mask_array[i])); } // read/compare phase: for(i=0;i<CNT;i++){ addr=addr_array[i]; if(skip_array[i]==1) continue; if(write_mask_array[i]==0x00000000) continue; if(read_mask_array[i]==0x00000000) continue; data_rd=(read_reg(addr) & read_mask_array[i]); wr_n=(write_mask_array[i] ^ 0xffffffff); exp_val=((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if(data_rd==exp_val) pass; else {wr_fail_cnt++; printf(\"Read_write : Failed : Write Read mismatch For Address %x, Expected value=0x%x\\tRead value=0x%x\\n\",addr,exp_val ,data_rd);} } 3) if(def_fail_cnt>0 || wr_fail_cnt>0) finish(1); else finish(0);",
      "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11, MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15, MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19, MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23, MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27, MIZAR_GPIO_GP0_GPIO_28, MIZAR_GPIO_GP0_GPIO_29, MIZAR_GPIO_GP0_GPIO_30, MIZAR_GPIO_GP0_GPIO_31, MIZAR_GPIO_GP0_GPIO_32, MIZAR_GPIO_GP0_GPIO_33, MIZAR_GPIO_GP0_GPIO_34, MIZAR_GPIO_GP0_GPIO_35, MIZAR_GPIO_GP0_GPIO_36, MIZAR_GPIO_GP0_GPIO_37, MIZAR_GPIO_GP0_GPIO_38, MIZAR_GPIO_GP0_GPIO_39, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_GPIO_GP0_INTR2_INTR_EN1, MIZAR_GPIO_GP0_INTR2_INTR_STS1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GPIO_DOUT_GROUP1, MIZAR_GPIO_GPIO_DOUT_GROUP2, MIZAR_GPIO_GPIO_DOUT_GROUP3, MIZAR_GPIO_GPIO_DOUT_GROUP4, MIZAR_GPIO_GPIO_DIN_GROUP1, MIZAR_GPIO_GPIO_DIN_GROUP2, MIZAR_GPIO_GPIO_DIN_GROUP3, MIZAR_GPIO_GPIO_DIN_GROUP4",
      "Hidden_Validation_Acceptance_Criteria": "Finish with 0 only if def_fail_cnt==0 and wr_fail_cnt==0; otherwise finish(1). Within loops: equality checks on masked default readings and masked pattern readbacks drive PASS/FAIL increments."
    },
    {
      "Index": "2",
      "SS / Module": "GPIO",
      "Feature": "Negative edge interrupt enable (neie) (Reset: 0x0)",
      "Test Case Name": "test_gpio_negedge_intr_en",
      "Test Description": "Validate falling-edge interrupt generation per GPIO pad 8–39 by configuring per-pin input mode with negative-edge enable, enabling group interrupts, generating a single falling edge per pin, and servicing/clearing latched raw and group status via GPIO and system registers.",
      "Speed": "NA",
      "Mode": "Interrupt",
      "Memory Start Offset": "0xA0243ffc",
      "Memory End Offset": "0xA0243ffc",
      "Remarks": "Timeout-based wait is used to avoid infinite hangs; the bound may need alignment with the simulation time base. Each iteration pre-clears raw status before enabling the respective group interrupt bit.",
      "Test Steps / Procedure": "Entry is through the test case followed by the interrupt handler when an interrupt occurs. 1) Enable the relevant system interrupt output by writing to LSS_SYSREG_INTR_EN1. 2) Drive the external pad driver to a known state high using address 0xA0243ffc. 3) Configure GPIO_GP0_GPIO_8..GPIO_GP0_GPIO_39 sequentially for input mode with negative-edge interrupt enabled and clear any per-pin raw status; insert a short wait after each configuration write. 4) For each bit position from 0 to 31: a) Pre-clear the corresponding raw status by writing the bit mask to GPIO_GPIO_INTR_RAW_STCLR1. b) Enable only that bit in GPIO_GP0_INTR1_INTR_EN1. c) Arm the wait flag prior to generating the edge. d) Generate a falling edge on that pad using the pad driver at address 0xA0243ffc (drive all high, wait briefly, then drive the target bit low). e) Poll with a bounded timeout until the interrupt is observed; on timeout, record an error. 5) In the interrupt service context: a) Deassert the wait flag and return the pad driver to all-high. b) Read the per-pin register for the current pad (GPIO_GP0_GPIO_8 plus offset) and verify the input bit reflects a low level after the falling edge. c) Confirm the per-pin raw status is set and the group masked status in GPIO_GP0_INTR1_INTR_STS1 has the corresponding bit set. d) Clear the per-pin raw status by writing a value that sets clear while maintaining input mode; also clear the corresponding group raw status via GPIO_GPIO_INTR_RAW_STCLR1. e) Verify the group masked status reads as zero. f) Clear the corresponding system raw status in LSS_SYSREG_RAW_STCR1 and clear the platform interrupt. 6) After servicing, proceed to the next pad until all pads are validated; return overall pass/fail based on accumulated errors.",
      "Impacted Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
      "Validation / Acceptance Criteria": "For each pad: 1) After the falling edge, the per-pin input field reports low. 2) The corresponding group masked status bit is set. 3) After clearing per-pin and group raw status, the group status reads as zero. 4) The system raw status is cleared. 5) No timeout occurs while waiting for the interrupt. Overall pass when no errors are recorded; otherwise fail.",
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
      "Hidden_Test_Description": "Falling-edge interrupt validation across GPIO[8..39] with ISR: configure doe=1, neie=1, iclr=1 per pin; enable group; generate 1->0 transition per pin via 0xA0243ffc; ISR checks DIN==0, raw set, group sts set, then clears iclr and RAW_STCLR1 and sysreg raw; bounded wait with timeout.",
      "Hidden_Remarks": "unsigned int timeout = 5000; // adjust to your sim time base if needed. Pre-clear raw status via write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val) prior to enabling the bit in MIZAR_GPIO_GP0_INTR1_INTR_EN1.",
      "Hidden_Test_Steps_Procedure": "Entry: int test_case() in program.c. 1) Conditionally GIC_EnableIRQ(87/88). 2) write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). 3) write_reg(0xA0243ffc, 0xffffffff). 4) for (i=0;i<32;i++): addr1=MIZAR_GPIO_GP0_GPIO_8 + (i*4); write_reg(addr1, (1u<<20)|(1u<<18)|(1u<<16)); wait_on(10). 5) for (i=0;i<32;i++): wr_val=1u<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val); timeout=5000; while(int_pend && timeout--) wait_on(10); if(timeout==0){ print(f"ERROR: Timeout waiting for GPIO{i+8} negedge interrupt"); test_err+=1 } 6) # finish(test_err) placeholder",
      "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
      "Hidden_Validation_Acceptance_Criteria": "Per iteration: (rdata & 0x1)==0 in ISR; (rdata & 0x2)!=0 and (read MIZAR_GPIO_GP0_INTR1_INTR_STS1 & (1<<i))!=0; after clears, group sts==0; system raw cleared; no timeout in wait loop; overall finish(test_err) with test_err==0 for pass."
    },
    {
      "Index": "3",
      "SS / Module": "GPIO",
      "Feature": "Positive edge interrupt enable (peie) (Reset: 0x0)",
      "Test Case Name": "test_gpio_pedge_all_pads_en",
      "Test Description": "Validate rising-edge interrupt generation for all GPIO pads 8–39 by enabling positive-edge detection per pin, configuring input mode via group IO control, enabling group interrupts, producing a single rising edge per pin, and servicing/clearing raw and masked status along with system interrupt status.",
      "Speed": "NA",
      "Mode": "Interrupt",
      "Memory Start Offset": "0xA0243ffc",
      "Memory End Offset": "0xA0243ffc",
      "Remarks": "The group interrupt is temporarily masked during service and re-enabled afterward. A bounded timeout prevents indefinite waiting for the interrupt on each pad.",
      "Test Steps / Procedure": "Entry is through the test case followed by the interrupt handler upon interrupt occurrence. 1) Enable the relevant system interrupt output by writing to LSS_SYSREG_INTR_EN1. 2) For each pin index 0..31, write to the per-pin control register (GPIO_GP0_GPIO_8 plus offset) to enable positive-edge interrupt. 3) Configure group IO control registers GPIO_GPIO_IO_CTRL_GROUP1..GPIO_GPIO_IO_CTRL_GROUP4 to set pads 8–39 to input mode. 4) Enable all group interrupts by writing all ones to GPIO_GP0_INTR1_INTR_EN1. 5) For each pin index 0..31: a) Set pad driver low using address 0xA0243ffc and wait briefly. b) Arm the wait flag. c) Drive the pad driver high to create a rising edge and wait until the interrupt is observed or timeout expires; on timeout, record an error and stop. d) Drive low again and wait before the next iteration. 6) In the interrupt service context: a) Capture the group masked status via GPIO_GP0_INTR1_INTR_STS1 and mask the group enable to avoid re-entry during service. b) If any bit is set, note success; otherwise record an error. c) Clear per-pin raw status by writing to each per-pin register (set clear bit) and wait briefly. d) Verify the group masked status reads as zero; on mismatch, record an error. e) Clear the system raw status in LSS_SYSREG_RAW_STCR1 and confirm it is cleared; on mismatch, record an error. f) Re-enable the group interrupt and clear the platform IRQ.",
      "Impacted Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
      "Validation / Acceptance Criteria": "A pad’s iteration passes when: 1) The group masked status indicates an interrupt occurred. 2) After issuing clear operations at the per-pin and group level, the group status reads as zero. 3) The system raw status bit is cleared after service. 4) No timeout occurs while waiting for the interrupt. Overall pass if no errors are recorded; otherwise fail.",
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
      "Hidden_Test_Description": "Rising-edge interrupt enable across all pads: per-pin write_reg(MIZAR_GPIO_GP0_GPIO_8 + i*4, 0x00020000) to set peie; set input mode via group IO CTRL; enable all in MIZAR_GPIO_GP0_INTR1_INTR_EN1; for each pin generate 0->1 edge using 0xA0243ffc and wait with timeout; ISR masks group, checks MIZAR_GPIO_GP0_INTR1_INTR_STS1, clears per-pin iclr via per-pin registers, verifies group clear, clears MIZAR_LSS_SYSREG_RAW_STCR1 and re-enables group, then clears GIC IRQ.",
      "Hidden_Remarks": "Default_IRQHandler masks GPIO group enable during service by writing 0x00000000 to MIZAR_GPIO_GP0_INTR1_INTR_EN1, then re-enables (0xFFFFFFFF) at end; bounded wait uses int timeout = 2000 with wait_on(10) loops.",
      "Hidden_Test_Steps_Procedure": "Entry: void test_case() in program.c. 1) GIC_EnableIRQ(87/88) per define. 2) write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR or LSS_SYSREG_INTR_EN1_GPIO1_INTR). 3) for(i=0;i<32;i++) write_reg(MIZAR_GPIO_GP0_GPIO_8 + (i*4), 0x00020000); wait_on(10). 4) write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF); wait_on(10). 5) write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF). 6) for(i=0;i<32;i++) { write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF); int timeout=2000; while((int_pend==1) && (--timeout>0)) wait_on(10); if(timeout==0){ print(f\"ERROR: Timeout waiting for GPIO IRQ at i={i}\\n\"); test_err+=1; break; } write_reg(0xA0243ffc, 0x00000000); wait_on(10); } # finish(test_err) placeholder",
      "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1, MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, MIZAR_GPIO_GP0_INTR1_INTR_EN1, MIZAR_GPIO_GP0_INTR1_INTR_STS1, MIZAR_LSS_SYSREG_RAW_STCR1",
      "Hidden_Validation_Acceptance_Criteria": "Per iteration: group masked status nonzero; after clearing per-pin raw across 32 pins, group status equals 0x0; LSS_SYSREG_RAW_STCR1 target bit clears; no timeout in wait loop; overall finish(test_err) with test_err==0 for pass."
    }
  ],
  "META_DATA": {
    "SourceRepo": "titusbspgit/PSVValidation",
    "Branch": "main",
    "Subdirectory": "TestRepo/gpio",
    "FoldersProcessedInOrder": [
      {
        "name": "gpio_reg_wr_rd_test",
        "github_url": "https://github.com/titusbspgit/PSVValidation/tree/main/TestRepo/gpio/gpio_reg_wr_rd_test",
        "files": [
          "TestRepo/gpio/gpio_reg_wr_rd_test/program.c",
          "TestRepo/gpio/gpio_reg_wr_rd_test/test_define.c",
          "TestRepo/gpio/gpio_reg_wr_rd_test/Makefile"
        ]
      },
      {
        "name": "test_gpio_negedge_intr_en",
        "github_url": "https://github.com/titusbspgit/PSVValidation/tree/main/TestRepo/gpio/test_gpio_negedge_intr_en",
        "files": [
          "TestRepo/gpio/test_gpio_negedge_intr_en/program.c",
          "TestRepo/gpio/test_gpio_negedge_intr_en/test_define.c",
          "TestRepo/gpio/test_gpio_negedge_intr_en/Makefile"
        ]
      },
      {
        "name": "test_gpio_pedge_all_pads_en",
        "github_url": "https://github.com/titusbspgit/PSVValidation/tree/main/TestRepo/gpio/test_gpio_pedge_all_pads_en",
        "files": [
          "TestRepo/gpio/test_gpio_pedge_all_pads_en/program.c",
          "TestRepo/gpio/test_gpio_pedge_all_pads_en/test_define.c",
          "TestRepo/gpio/test_gpio_pedge_all_pads_en/Makefile"
        ]
      }
    ],
    "FeatureRAGSource": "Rg-Emb-Mpsoc-Features",
    "MacroRegisterMappingSource": "Rg-Emb-Mpsoc-Macro-Reg-Map",
    "Notes": "All values are derived strictly from source code. Main text fields (Description, Remarks, Steps, Validation) are rewritten from hidden META using macro-to-register mapping; MIZAR macro identifiers are preserved only in Hidden_* and Impacted Registers as required. Timing values, delays, and timeouts are included when explicitly present in source."
  }
}'''

def main():
    try:
        obj = json.loads(JSON_DATA)
    except Exception as e:
        raise SystemExit(f"Invalid JSON input: {e}")

    # Extract tabular data from TestCases when available
    if isinstance(obj, dict) and isinstance(obj.get("TestCases"), list):
        records = obj["TestCases"]
    elif isinstance(obj, list):
        records = obj
    elif isinstance(obj, dict):
        records = [obj]
    else:
        raise SystemExit("Unsupported JSON structure for tabular conversion")

    # Union of keys in first-seen order
    keys = []
    seen = set()
    for rec in records:
        if isinstance(rec, dict):
            for k in rec.keys():
                if k not in seen:
                    seen.add(k)
                    keys.append(k)

    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Header row
    for c, k in enumerate(keys, start=1):
        cell = ws.cell(row=1, column=c, value=k)
        cell.font = Font(bold=True)
    ws.freeze_panes = "A2"

    # Data rows
    for r, rec in enumerate(records, start=2):
        for c, k in enumerate(keys, start=1):
            ws.cell(row=r, column=c, value=rec.get(k, ""))

    # Basic width sizing for Data
    for c in range(1, ws.max_column + 1):
        maxlen = 0
        for r in range(1, ws.max_row + 1):
            v = ws.cell(row=r, column=c).value
            maxlen = max(maxlen, len(str(v)) if v is not None else 0)
        ws.column_dimensions[get_column_letter(c)].width = min(maxlen + 2, 100)

    # Create Meta_data_sheet and copy META columns
    meta_cols = [
        "Hidden_Test_Case_Name",
        "Hidden_Test_Description",
        "Hidden_Remarks",
        "Hidden_Test_Steps_Procedure",
        "Hidden_Impacted_Registers",
        "Hidden_Validation_Acceptance_Criteria",
    ]
    meta = wb.create_sheet("Meta_data_sheet")
    for c, k in enumerate(meta_cols, start=1):
        meta.cell(row=1, column=c, value=k)
    for r, rec in enumerate(records, start=2):
        for c, k in enumerate(meta_cols, start=1):
            meta.cell(row=r, column=c, value=rec.get(k, ""))
    meta.sheet_state = "veryHidden"

    # Prepare TestPlan sheet by removing META cols and reordering
    testplan = ws
    testplan.title = "TestPlan"

    # Remove META columns from TestPlan (if present)
    meta_present = [k for k in meta_cols if k in keys]
    # Delete by index from right to left
    for k in sorted(meta_present, key=lambda x: keys.index(x), reverse=True):
        idx = keys.index(k) + 1
        testplan.delete_cols(idx)
        keys.remove(k)

    final_order = [
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

    # Build new ordered sheet
    tmp = wb.create_sheet("TMP")
    for c, name in enumerate(final_order, start=1):
        tmp.cell(row=1, column=c, value=name)
    header_src = [testplan.cell(row=1, column=c).value for c in range(1, testplan.max_column + 1)]
    header_map = {name: i + 1 for i, name in enumerate(header_src)}

    for r in range(2, testplan.max_row + 1):
        for c, name in enumerate(final_order, start=1):
            src_col = header_map.get(name, None)
            val = testplan.cell(row=r, column=src_col).value if src_col else ""
            tmp.cell(row=r, column=c, value=val)

    wb.remove(testplan)
    tmp.title = "TestPlan"
    testplan = tmp
    testplan.freeze_panes = "A2"

    # Strict formatting on TestPlan
    header_fill = PatternFill("solid", fgColor="4472C4")
    header_font = Font(bold=True, color="FFFFFF")
    header_align = Alignment(horizontal="center", vertical="center")

    for c in range(1, testplan.max_column + 1):
        cell = testplan.cell(row=1, column=c)
        cell.fill = header_fill
        cell.font = header_font
        cell.alignment = header_align

    # Wrap text for specific columns
    wrap_cols = [
        "Test Description",
        "Remarks",
        "Test Steps / Procedure",
        "Validation / Acceptance Criteria",
    ]
    wrap_idx = []
    for name in wrap_cols:
        for c in range(1, testplan.max_column + 1):
            if testplan.cell(row=1, column=c).value == name:
                wrap_idx.append(c)
                break

    for r in range(2, testplan.max_row + 1):
        for c in wrap_idx:
            testplan.cell(row=r, column=c).alignment = Alignment(wrap_text=True, vertical="top", horizontal="left")

    # Default alignment for other data cells
    index_col = None
    for c in range(1, testplan.max_column + 1):
        if testplan.cell(row=1, column=c).value == "Index":
            index_col = c
            break
    for r in range(2, testplan.max_row + 1):
        for c in range(1, testplan.max_column + 1):
            if c == index_col:
                testplan.cell(row=r, column=c).alignment = Alignment(horizontal="center", vertical="top")
            elif c in wrap_idx:
                continue
            else:
                testplan.cell(row=r, column=c).alignment = Alignment(horizontal="left", vertical="top")

    # Borders for all populated cells
    thin = Side(style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for r in range(1, testplan.max_row + 1):
        for c in range(1, testplan.max_column + 1):
            testplan.cell(row=r, column=c).border = border

    # Column widths based on content length
    for c in range(1, testplan.max_column + 1):
        maxlen = 0
        for r in range(1, testplan.max_row + 1):
            v = testplan.cell(row=r, column=c).value
            maxlen = max(maxlen, len(str(v)) if v is not None else 0)
        testplan.column_dimensions[get_column_letter(c)].width = min(maxlen + 2, 120)

    # Data validation for Code Generation column
    code_col = None
    for c in range(1, testplan.max_column + 1):
        if testplan.cell(row=1, column=c).value == "Code Generation (Required / Not)":
            code_col = c
            break
    if code_col:
        dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showDropDown=True)
        rng = f"{get_column_letter(code_col)}2:{get_column_letter(code_col)}{testplan.max_row}"
        dv.add(rng)
        testplan.add_data_validation(dv)

    # Save with IST timestamp-based filename rule
    base_dir = os.environ.get("OUTPUT_DIR", "Test_Output/GPIO/TestPlan").strip()
    if not base_dir:
        base_dir = "Test_Output/GPIO/TestPlan"
    os.makedirs(base_dir, exist_ok=True)

    ist = timezone(timedelta(hours=5, minutes=30))
    now = datetime.now(tz=ist)
    ip_name = os.environ.get("IP_NAME", "GPIO")
    file_name = f"{ip_name}_TestPlan_{now.strftime('%Y%m%d')}_{now.strftime('%H%M%S')}.xlsx"

    out_path = os.path.join(base_dir, file_name)
    wb.save(out_path)
    print(out_path)

if __name__ == "__main__":
    main()
