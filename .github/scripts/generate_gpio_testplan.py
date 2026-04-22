import json
import os
from datetime import datetime
from zoneinfo import ZoneInfo
from copy import deepcopy
from openpyxl import Workbook
from openpyxl.styles import Alignment, Border, Side, Font, PatternFill
from openpyxl.worksheet.datavalidation import DataValidation

# JSON payload embedded exactly as provided
JSON_INPUT = r'''{
  "ip": "GPIO",
  "repo": "titusbspgit/PSVValidation",
  "branch": "main",
  "subdir": "TestRepo/gpio",
  "generated_at_tz": "IST",
  "test_cases": [
    {
      "Index": 1,
      "SS / Module": "GPIO",
      "Feature": "Independent control register to configure interrupt type, data to be driven in output mode, GPIO input data status in input mode and GPIO IO mode control",
      "Test Case Name": "gpio_reg_wr_rd_test",
      "Test Description": "Validates default values and masked write/read behavior across per-pin control and group GPIO registers by comparing masked readbacks to expected defaults and pattern-composed values.",
      "Speed": "NA",
      "Mode": "NA",
      "Memory Start Offset": "NA",
      "Memory End Offset": "NA",
      "Remarks": "Default input status may read high unless external drive conditions are forced low; certain volatile write/read registers are skipped per test configuration; diagnostic prints may be conditionally compiled.",
      "Test Steps / Procedure": [
        "Enter the test and perform a default-value sweep across the per-pin control registers for pins 8 through 39 and the listed group/interrupt registers using the provided address table.",
        "For each address in the table, if the default-read mask is zero, skip reading; otherwise perform a read and mask off the input status bit as defined, then compare to the expected default value.",
        "Accumulate default mismatches as failures.",
        "Iterate through six write patterns and, for each pattern, traverse every address in the table.",
        "For each address, if the write is configured to be skipped or the write mask is zero, skip the write; otherwise write the pattern masked by the write mask.",
        "After completing writes for a given pattern, traverse the address table again to read back results.",
        "For each address, if writing was skipped or masks prohibit read, skip; otherwise read and apply the read mask, then compute the expected value by combining masked write bits with masked default-preserve bits, and compare.",
        "Accumulate any write/read mismatches as failures.",
        "Upon completion of all patterns and addresses, declare pass only if both default and write/read failure counters are zero; otherwise declare fail."
      ],
      "Impacted Registers": "GP0_GPIO_8,GP0_GPIO_9,GP0_GPIO_10,GP0_GPIO_11,GP0_GPIO_12,GP0_GPIO_13,GP0_GPIO_14,GP0_GPIO_15,GP0_GPIO_16,GP0_GPIO_17,GP0_GPIO_18,GP0_GPIO_19,GP0_GPIO_20,GP0_GPIO_21,GP0_GPIO_22,GP0_GPIO_23,GP0_GPIO_24,GP0_GPIO_25,GP0_GPIO_26,GP0_GPIO_27,GP0_GPIO_28,GP0_GPIO_29,GP0_GPIO_30,GP0_GPIO_31,GP0_GPIO_32,GP0_GPIO_33,GP0_GPIO_34,GP0_GPIO_35,GP0_GPIO_36,GP0_GPIO_37,GP0_GPIO_38,GP0_GPIO_39,GPIO_INTR_RAW_STCLR1,GP0_INTR1_INTR_EN1,GP0_INTR1_INTR_STS1,GP0_INTR2_INTR_EN1,GP0_INTR2_INTR_STS1,GPIO_IO_CTRL_GROUP1,GPIO_IO_CTRL_GROUP2,GPIO_IO_CTRL_GROUP3,GPIO_IO_CTRL_GROUP4,GPIO_DOUT_GROUP1,GPIO_DOUT_GROUP2,GPIO_DOUT_GROUP3,GPIO_DOUT_GROUP4,GPIO_DIN_GROUP1,GPIO_DIN_GROUP2,GPIO_DIN_GROUP3,GPIO_DIN_GROUP4",
      "Validation / Acceptance Criteria": [
        "For the default-value sweep, each masked read must exactly match the corresponding expected default value.",
        "For each write pattern and address, the masked readback value must equal the composition of the written pattern on writable bits with default-preserved values on non-writable bits.",
        "Overall pass requires both default and write/read failure counters to be zero; any mismatch results in fail."
      ],
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "gpio_reg_wr_rd_test",
      "Hidden_Test_Description": "Checks default values and write/read for all registers listed in addr_array using chk_rst_val() and chk_rd_wr(). Default sweep masks LSB (data = data_rd & 0xfffffffe) before comparing to default_value_array[i]. Write/read iterates six patterns, writes (data_wr & write_mask_array[i]) and verifies readbacks: exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | ((~write_mask_array[i]) & read_mask_array[i] & default_value_array[i])). Finish(0) iff def_fail_cnt and wr_fail_cnt are zero.",
      "Hidden_Remarks": "Comment: //when reading default values the din value is becoming 1 automatically if we don't force any value,but if we force zero to din bit level sel becoming high,so that reding value not matched with expected value. Comment: //SKIPPING VRRW registers (skip_array configured). DEBUG_DISPLAY prints guarded by macro.",
      "Hidden_Test_Steps_Procedure": [
        "test_case(): calls chk_rst_val(); then chk_rd_wr(); if (def_fail_cnt > 0 || wr_fail_cnt > 0) finish(1); else finish(0).",
        "chk_rst_val(): for (i=0; i<CNT; i++) { addr = addr_array[i]; if (skip_rst_array[i]==1) continue; if (read_mask_array[i]==0) continue; data_rd = read_reg(addr); data = (data_rd & 0xfffffffe); if (data == default_value_array[i]) {pass} else {def_fail_cnt++; printf failure}; }",
        "chk_rd_wr(): unsigned chk_val[6] = {0xffffffff,0xaaaaaaaa,0x55555555,0xf5f5f5f5,0xA5A5A5A5,0xffff0000}; for (j=0; j<6; j++) { data_wr = chk_val[j]; // write phase",
        "for (i=0; i<CNT; i++) { addr = addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0) continue; write_reg(addr, (data_wr & write_mask_array[i])); }",
        "// read/verify phase",
        "for (i=0; i<CNT; i++) { addr = addr_array[i]; if (skip_array[i]==1) continue; if (write_mask_array[i]==0) continue; if (read_mask_array[i]==0) continue; data_rd = (read_reg(addr) & read_mask_array[i]); wr_n = (write_mask_array[i] ^ 0xffffffff); exp_val = ((data_wr & read_mask_array[i] & write_mask_array[i]) | (wr_n & read_mask_array[i] & default_value_array[i])); if (data_rd == exp_val) {pass} else {wr_fail_cnt++; printf mismatch}; } }",
        "soft_reset_chk(): compiled out (#ifdef 0)."
      ],
      "Hidden_Impacted_Registers": "MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_GPIO_9,MIZAR_GPIO_GP0_GPIO_10,MIZAR_GPIO_GP0_GPIO_11,MIZAR_GPIO_GP0_GPIO_12,MIZAR_GPIO_GP0_GPIO_13,MIZAR_GPIO_GP0_GPIO_14,MIZAR_GPIO_GP0_GPIO_15,MIZAR_GPIO_GP0_GPIO_16,MIZAR_GPIO_GP0_GPIO_17,MIZAR_GPIO_GP0_GPIO_18,MIZAR_GPIO_GP0_GPIO_19,MIZAR_GPIO_GP0_GPIO_20,MIZAR_GPIO_GP0_GPIO_21,MIZAR_GPIO_GP0_GPIO_22,MIZAR_GPIO_GP0_GPIO_23,MIZAR_GPIO_GP0_GPIO_24,MIZAR_GPIO_GP0_GPIO_25,MIZAR_GPIO_GP0_GPIO_26,MIZAR_GPIO_GP0_GPIO_27,MIZAR_GPIO_GP0_GPIO_28,MIZAR_GPIO_GP0_GPIO_29,MIZAR_GPIO_GP0_GPIO_30,MIZAR_GPIO_GP0_GPIO_31,MIZAR_GPIO_GP0_GPIO_32,MIZAR_GPIO_GP0_GPIO_33,MIZAR_GPIO_GP0_GPIO_34,MIZAR_GPIO_GP0_GPIO_35,MIZAR_GPIO_GP0_GPIO_36,MIZAR_GPIO_GP0_GPIO_37,MIZAR_GPIO_GP0_GPIO_38,MIZAR_GPIO_GP0_GPIO_39,MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_GPIO_GP0_INTR2_INTR_EN1,MIZAR_GPIO_GP0_INTR2_INTR_STS1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,MIZAR_GPIO_GPIO_IO_CTRL_GROUP4,MIZAR_GPIO_GPIO_DOUT_GROUP1,MIZAR_GPIO_GPIO_DOUT_GROUP2,MIZAR_GPIO_GPIO_DOUT_GROUP3,MIZAR_GPIO_GPIO_DOUT_GROUP4,MIZAR_GPIO_GPIO_DIN_GROUP1,MIZAR_GPIO_GPIO_DIN_GROUP2,MIZAR_GPIO_GPIO_DIN_GROUP3,MIZAR_GPIO_GPIO_DIN_GROUP4",
      "Hidden_Validation_Acceptance_Criteria": "Pass if def_fail_cnt == 0 and wr_fail_cnt == 0 at end of test_case(); comparisons performed in chk_rst_val() and chk_rd_wr() must hold for all addresses/patterns; any mismatch increments respective counters and results in finish(1)."
    },
    {
      "Index": 2,
      "SS / Module": "GPIO",
      "Feature": "Interrupts can be generated based on positive edge or negative edge or level high or level low detection at GPIO input.",
      "Test Case Name": "test_gpio_negedge_intr_en",
      "Test Description": "Verifies negative-edge interrupt functionality for GPIO pins 8–39 by configuring input mode with negedge detection, generating falling edges per pin, and validating pin-level and group interrupt status, clearing, and system interrupt deassertion.",
      "Speed": "NA",
      "Mode": "Interrupt",
      "Memory Start Offset": "0xA0243ffc",
      "Memory End Offset": "0xA0243ffc",
      "Remarks": "A bounded interrupt wait with timeout is used to avoid infinite loops and is tuned via a fixed iteration count; the wait is armed before edge generation to avoid race conditions; initial pad driver level is forced high to establish a known state.",
      "Test Steps / Procedure": [
        "Initialize error counter to zero.",
        "Enable the appropriate external interrupt line in the interrupt controller for the targeted GPIO instance.",
        "Enable the corresponding system register interrupt output for the targeted GPIO instance.",
        "Drive the associated pad driver register to all-high to establish a known initial level.",
        "For each pin from 8 to 39, program the per-pin control to input mode, enable negative-edge detection, and clear any latched raw status, inserting a short wait after each write.",
        "For each bit position from 0 to 31, clear the corresponding raw interrupt bit in the raw status/clear register at the group level.",
        "Enable only that bit in the masked interrupt enable register for the group and insert a short wait.",
        "Arm the interrupt-wait flag before creating the stimulus edge.",
        "Generate a falling edge on the selected pin by first driving all pads high, waiting briefly, then driving all pads low except the selected bit.",
        "Wait for the service routine to clear the pending flag, enforcing a finite timeout to prevent hangs; on timeout, record an error.",
        "In the interrupt service routine: clear the pending flag and restore pad driver to high.",
        "Read the per-pin control/status and validate that the input status bit is low after the falling edge; record an error if not.",
        "Check that the per-pin raw interrupt indicator is asserted and that the corresponding bit is set in the masked group interrupt status; record an error if not.",
        "Clear the per-pin raw condition by writing the per-pin clear while keeping input mode, and also clear the group raw status bit.",
        "Verify that the masked group interrupt status reads as zero after clearing; record an error if not.",
        "Clear the corresponding system interrupt raw status and clear the external interrupt controller line for the targeted instance."
      ],
      "Impacted Registers": "INTR_EN1,RAW_STCR1,GP0_GPIO_8,GPIO_INTR_RAW_STCLR1,GP0_INTR1_INTR_EN1,GP0_INTR1_INTR_STS1",
      "Validation / Acceptance Criteria": [
        "For each pin, an interrupt must be observed within the configured timeout after generating a falling edge; lack of service before timeout is a failure.",
        "Within the interrupt service, the per-pin input status must be low following the falling edge.",
        "The masked group interrupt status must indicate the serviced pin prior to clear, and must read zero after clearing the per-pin and group raw conditions.",
        "The system interrupt raw status must be cleared successfully for the targeted GPIO instance.",
        "Overall pass requires zero accumulated errors at test end."
      ],
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "test_gpio_negedge_intr_en",
      "Hidden_Test_Description": "Negative-edge interrupt enable/flow test for GPIO[8..39]: config doe=1 (input), neie=1, iclr=1 per pin; per bit loop: clear RAW_STCLR1, enable only that bit in INTR_EN1, arm int_pend=1, drive 0xA0243ffc high then ~wr_val to create negedge, wait until int_pend cleared or timeout; ISR checks DIN==0, (rdata & 0x2)!=0 -> group STS has bit set; clear per-pin via (doe=1|iclr=1) and group RAW_STCLR1; verify group STS==0; clear sysreg RAW_STCR1 and GIC IRQ; errors increment test_err; finish(test_err).",
      "Hidden_Remarks": "Comments: \"Arm the wait BEFORE generating the edge to avoid race\"; \"Bounded wait instead of infinite loop\"; \"Drive all high initially (known state)\"; Timeout value noted as 5000 with a comment to adjust to sim time base.",
      "Hidden_Test_Steps_Procedure": [
        "test_case(): test_err=0; #ifdef GPIO0 GIC_EnableIRQ(87); #ifdef GPIO1 GIC_EnableIRQ(88);",
        "#ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);",
        "write_reg(0xA0243ffc, 0xffffffff);",
        "for (i=0;i<32;i++){ addr1=MIZAR_GPIO_GP0_GPIO_8+(i*4); write_reg(addr1, (1u<<20)|(1u<<18)|(1u<<16)); wait_on(10); }",
        "for (i=0;i<32;i++){ wr_val=1u<<i; write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, wr_val); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, wr_val); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xffffffff); wait_on(30); write_reg(0xA0243ffc, ~wr_val); unsigned int timeout=5000; while(int_pend && timeout--){ wait_on(10);} if(timeout==0){ printf(\"ERROR: Timeout...\"); test_err++; } }",
        "finish(test_err);",
        "Default_IRQHandler(): unsigned int local_wr=1u<<i; int_pend=0; write_reg(0xA0243ffc, 0xffffffff); raddr=MIZAR_GPIO_GP0_GPIO_8+(i*4); rdata=read_reg(raddr); if((rdata & 0x1)!=0){ test_err++; }",
        "if((rdata & 0x2)!=0x0){ rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if((rdata_grp & local_wr)==0){ test_err++; } raddr2=MIZAR_GPIO_GP0_GPIO_8+(i*4); write_reg(raddr2, (1u<<20)|(1u<<16)); write_reg(MIZAR_GPIO_GPIO_INTR_RAW_STCLR1, local_wr); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if(rdata_grp!=0x0){ test_err++; } #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); GIC_ClearIRQ(87); #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); GIC_ClearIRQ(88); #endif } else { test_err++; }"
      ],
      "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1,MIZAR_LSS_SYSREG_RAW_STCR1,MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1",
      "Hidden_Validation_Acceptance_Criteria": "No timeouts while waiting for ISR; within ISR DIN (bit0) must be 0; per-pin raw (bit1) observed; masked group status has the bit set pre-clear and becomes 0 post-clear; system RAW_STCR1 bit cleared; overall pass is test_err==0."
    },
    {
      "Index": 3,
      "SS / Module": "GPIO",
      "Feature": "Interrupts can be generated based on positive edge or negative edge or level high or level low detection at GPIO input.",
      "Test Case Name": "test_gpio_pedge_all_pads_en",
      "Test Description": "Verifies positive-edge interrupt functionality on all GPIO pins 8–39 by enabling posedge detection, placing pins in input mode, enabling all masked interrupts, generating rising edges per pin, and validating group interrupt assertion and clearing along with system interrupt deassertion.",
      "Speed": "NA",
      "Mode": "Interrupt",
      "Memory Start Offset": "0xA0243ffc",
      "Memory End Offset": "0xA0243ffc",
      "Remarks": "Group interrupt output is masked during service to prevent re-entrancy, then re-enabled; a bounded wait with timeout guards the polling for ISR completion; per-pin raw conditions are cleared for all pins after each service.",
      "Test Steps / Procedure": [
        "Enable the appropriate external interrupt line in the interrupt controller according to the targeted GPIO instance.",
        "Enable the corresponding system register interrupt output for the targeted GPIO instance.",
        "For each pin from 8 to 39, program the per-pin control to enable positive-edge detection.",
        "Insert a short delay to let configuration settle.",
        "Configure the group IO control registers to place the pins in input mode.",
        "Insert a short delay to let IO mode settle.",
        "Enable all masked interrupts for the group.",
        "For each bit position from 0 to 31, prepare a low level on the pad driver, arm the interrupt-wait flag, and generate a rising edge by driving the pad driver high.",
        "Poll with a finite timeout for the interrupt-wait flag to be cleared by the service routine; on timeout, record an error and stop.",
        "Optionally return the pad driver low again and delay before the next iteration.",
        "In the interrupt service routine, compute the current bit index mask and clear the pending flag.",
        "Read the masked group interrupt status; temporarily mask group output during service.",
        "If group status is nonzero, continue; otherwise record an error.",
        "Clear raw per-pin conditions by writing the per-pin raw-clear field for all pins and delay briefly.",
        "Verify that masked group interrupt status reads as zero after clearing; record an error if not.",
        "Clear the corresponding system interrupt raw status for the targeted instance and verify that the status is deasserted.",
        "Re-enable the masked group interrupt output and clear the external interrupt controller line."
      ],
      "Impacted Registers": "INTR_EN1,RAW_STCR1,GP0_GPIO_8,GP0_INTR1_INTR_EN1,GP0_INTR1_INTR_STS1,GPIO_IO_CTRL_GROUP1,GPIO_IO_CTRL_GROUP2,GPIO_IO_CTRL_GROUP3,GPIO_IO_CTRL_GROUP4",
      "Validation / Acceptance Criteria": [
        "For each pin, an interrupt must be observed within the configured timeout following a rising edge; a timeout constitutes failure.",
        "During service, masked group interrupt status must be asserted and must return to zero after clearing per-pin raw conditions.",
        "The relevant system interrupt raw status must be cleared and remain deasserted after the service.",
        "Overall pass requires zero accumulated errors at test completion."
      ],
      "Code Generation (Required / Not)": "",
      "Hidden_Test_Case_Name": "test_gpio_pedge_all_pads_en",
      "Hidden_Test_Description": "Positive-edge interrupt test for GPIO[8..39]: enable GIC IRQ (87/88); enable sysreg GPIOx interrupt; for i in 0..31: write per-pin (base + i*4) 0x00020000 (peie=1); wait; set IO_CTRL_GROUP1..4 = 0x000000FF (doe=1); wait; enable group INTR_EN1 = 0xFFFFFFFF; loop i: write 0xA0243ffc=0x0; wait; int_pend=1; write 0xA0243ffc=0xFFFFFFFF (posedge); wait until !int_pend or timeout (2000) else error; write 0xA0243ffc=0; wait; finish(test_err). ISR: wr_val=1<<i; int_pend=0; rdata_grp=read(INTR_STS1); write(INTR_EN1, 0x0); if ((rdata_grp & 0xffffffff)!=0) ok else error; for j in 0..31: write (base + j*4) 0x00010000 (iclr=1); wait_on(2); rdata_grp=read(INTR_STS1); if (rdata_grp==0) ok else error; clear sysreg RAW_STCR1 for GPIOx and verify cleared via readback; write(INTR_EN1, 0xFFFFFFFF); GIC_ClearIRQ(87/88).",
      "Hidden_Remarks": "Group masked during service via write to group interrupt enable register set to 0; finite timeout loop (2000) guards ISR wait; per-pin raw clear performed for all pins; optional debug prints under DEBUG_DISPLAY.",
      "Hidden_Test_Steps_Procedure": [
        "test_case(): #ifdef GPIO0 GIC_EnableIRQ(87); #ifdef GPIO1 GIC_EnableIRQ(88);",
        "#ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO0_INTR); #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIO1_INTR);",
        "for(i=0;i<32;i++){ write_reg(MIZAR_GPIO_GP0_GPIO_8+(i*4), 0x00020000); } wait_on(10);",
        "write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, 0x000000FF); write_reg(MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, 0x000000FF); wait_on(10);",
        "write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF);",
        "for(i=0;i<32;i++){ write_reg(0xA0243ffc, 0x00000000); wait_on(10); int_pend=1; write_reg(0xA0243ffc, 0xFFFFFFFF); int timeout=2000; while((int_pend==1) && (--timeout>0)){ wait_on(10);} if(timeout==0){ printf(\"ERROR: Timeout...\"); test_err++; break;} write_reg(0xA0243ffc, 0x00000000); wait_on(10);} finish(test_err);",
        "Default_IRQHandler(): wr_val=1<<i; int_pend=0; rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0x00000000); if((rdata_grp & 0xffffffff)!=0){ /* ok */ } else { printf(\"ERROR: Group Interrupt not occured\"); test_err++; } for(j=0;j<32;j++){ write_reg(MIZAR_GPIO_GP0_GPIO_8+(j*4), 0x00010000);} wait_on(2); rdata_grp=read_reg(MIZAR_GPIO_GP0_INTR1_INTR_STS1); if(rdata_grp==0x0){ /* ok */ } else { printf(\"ERROR : Group Interrupt clear failed\"); test_err++; } #ifdef GPIO0 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO0_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if((rdata & LSS_SYSREG_RAW_STCR1_GPIO0_INTR)!=0){ printf(\"sysreg status not cleared\"); test_err++; } #endif #ifdef GPIO1 write_reg(MIZAR_LSS_SYSREG_RAW_STCR1, LSS_SYSREG_RAW_STCR1_GPIO1_INTR); rdata=read_reg(MIZAR_LSS_SYSREG_RAW_STCR1); if((rdata & LSS_SYSREG_RAW_STCR1_GPIO1_INTR)!=0){ printf(\"sysreg status not cleared\"); test_err++; } #endif write_reg(MIZAR_GPIO_GP0_INTR1_INTR_EN1, 0xFFFFFFFF); #ifdef GPIO0 GIC_ClearIRQ(87); #endif #ifdef GPIO1 GIC_ClearIRQ(88); #endif"
      ],
      "Hidden_Impacted_Registers": "MIZAR_LSS_SYSREG_INTR_EN1,MIZAR_LSS_SYSREG_RAW_STCR1,MIZAR_GPIO_GP0_GPIO_8,MIZAR_GPIO_GP0_INTR1_INTR_EN1,MIZAR_GPIO_GP0_INTR1_INTR_STS1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP1,MIZAR_GPIO_GPIO_IO_CTRL_GROUP2,MIZAR_GPIO_GPIO_IO_CTRL_GROUP3,MIZAR_GPIO_GPIO_IO_CTRL_GROUP4",
      "Hidden_Validation_Acceptance_Criteria": "Each iteration must observe ISR within timeout; group masked status nonzero on entry and becomes zero after per-pin raw clear; system RAW_STCR1 bit cleared; test_err must be zero at end."
    }
  ]
}'''

MAIN_COLS = [
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

META_COLS = [
    "Hidden_Test_Case_Name",
    "Hidden_Test_Description",
    "Hidden_Remarks",
    "Hidden_Test_Steps_Procedure",
    "Hidden_Impacted_Registers",
    "Hidden_Validation_Acceptance_Criteria",
]

OUTPUT_DIR = os.path.join("Test_Output", "GPIO", "TestPlan")
IP_NAME = "GPIO"


def to_cell_value(v):
    if isinstance(v, list):
        return "\n".join(str(x) for x in v)
    return v


def union_keys_preserve_order(rows):
    seen = set()
    order = []
    for row in rows:
        for k in row.keys():
            if k not in seen:
                seen.add(k)
                order.append(k)
    return order


def auto_fit_columns(ws):
    # approximate width: max length of string in column + padding
    for col_cells in ws.columns:
        max_len = 0
        col_letter = col_cells[0].column_letter
        for c in col_cells:
            val = c.value
            if val is None:
                continue
            if isinstance(val, (int, float)):
                l = len(str(val))
            else:
                s = str(val)
                l = max(len(part) for part in s.splitlines()) if s else 0
            if l > max_len:
                max_len = l
        width = min(max(10, max_len + 2), 100)
        ws.column_dimensions[col_letter].width = width


def apply_borders(ws):
    thin = Side(border_style="thin", color="000000")
    border = Border(left=thin, right=thin, top=thin, bottom=thin)
    for row in ws.iter_rows(min_row=1, max_row=ws.max_row, min_col=1, max_col=ws.max_column):
        for cell in row:
            cell.border = border


def build_workbook(data_rows):
    wb = Workbook()
    ws = wb.active
    ws.title = "Data"

    # Deduplicate by Test Case Name (stable)
    seen = set()
    deduped = []
    for r in data_rows:
        name = r.get("Test Case Name")
        if name in seen:
            continue
        seen.add(name)
        deduped.append(r)

    # Normalize keys
    keys = union_keys_preserve_order(deduped)

    # Write header
    for j, k in enumerate(keys, start=1):
        cell = ws.cell(row=1, column=j, value=k)
        cell.font = Font(bold=True)
        cell.alignment = Alignment(horizontal="center", vertical="center")

    # Write rows
    for i, row in enumerate(deduped, start=2):
        for j, k in enumerate(keys, start=1):
            ws.cell(row=i, column=j, value=to_cell_value(row.get(k, "")))

    # Freeze top row
    ws.freeze_panes = "A2"

    # Autofit
    auto_fit_columns(ws)

    # Create Meta_data_sheet
    meta = wb.create_sheet("Meta_data_sheet")
    for j, k in enumerate(META_COLS, start=1):
        meta.cell(row=1, column=j, value=k).font = Font(bold=True)
    for i, row in enumerate(deduped, start=2):
        for j, k in enumerate(META_COLS, start=1):
            meta.cell(row=i, column=j, value=to_cell_value(row.get(k, "")))
    # Very hidden
    meta.sheet_state = 'veryHidden'

    # Prepare TestPlan sheet from Data
    ws.title = "TestPlan"
    # Build visible columns in required order
    tp = wb.create_sheet("_tp_tmp_")
    for j, k in enumerate(MAIN_COLS, start=1):
        h = tp.cell(row=1, column=j, value=k)
        h.font = Font(bold=True)
        h.alignment = Alignment(horizontal="center", vertical="center")
    # Map from Data headers to column index
    data_headers = [c.value for c in ws[1]]
    header_index = {h: idx+1 for idx, h in enumerate(data_headers)}

    # Copy rows while excluding META columns and reordering
    for i in range(2, ws.max_row + 1):
        for j, k in enumerate(MAIN_COLS, start=1):
            src_col = header_index.get(k)
            val = ""
            if src_col is not None:
                val = ws.cell(row=i, column=src_col).value
            tp.cell(row=i, column=j, value=val)

    # Replace TestPlan with tmp
    wb.remove(ws)
    tp.title = "TestPlan"

    # Formatting for TestPlan
    header_fill = PatternFill("solid", fgColor="4472C4")
    for cell in tp[1]:
        cell.fill = header_fill
        cell.font = Font(bold=True, color="FFFFFF")
        cell.alignment = Alignment(horizontal="center", vertical="center")

    wrap_cols = {
        "Test Description",
        "Remarks",
        "Test Steps / Procedure",
        "Validation / Acceptance Criteria",
    }
    # Determine column indices
    tp_headers = [c.value for c in tp[1]]
    wrap_idx = {tp_headers.index(c)+1 for c in tp_headers if c in wrap_cols}
    # Apply alignments
    for row in tp.iter_rows(min_row=2, max_row=tp.max_row, min_col=1, max_col=tp.max_column):
        for cell in row:
            # Default vertical top
            if cell.column == 1:  # Index
                cell.alignment = Alignment(horizontal="center", vertical="top", wrap_text=False)
            elif cell.column in wrap_idx:
                cell.alignment = Alignment(horizontal="left", vertical="top", wrap_text=True)
            else:
                cell.alignment = Alignment(horizontal="left", vertical="top", wrap_text=False)

    # Autofit and borders
    auto_fit_columns(tp)
    apply_borders(tp)

    # Freeze header
    tp.freeze_panes = "A2"

    # Data validation for Code Generation (Required / Not)
    if "Code Generation (Required / Not)" in tp_headers:
        col_idx = tp_headers.index("Code Generation (Required / Not)") + 1
        dv = DataValidation(type="list", formula1='"Required,Not Required"', allow_blank=True, showErrorMessage=True)
        rng = f"{tp.cell(row=2, column=col_idx).coordinate}:{tp.cell(row=tp.max_row, column=col_idx).coordinate}"
        dv.add(rng)
        tp.add_data_validation(dv)

    return wb


def main():
    data = json.loads(JSON_INPUT)
    rows = deepcopy(data.get("test_cases", []))

    # Build workbook
    wb = build_workbook(rows)

    # Compute IST timestamp and filename
    ist = datetime.now(ZoneInfo("Asia/Kolkata"))
    ts_date = ist.strftime("%Y%m%d")
    ts_time = ist.strftime("%H%M%S")
    ist_pretty = ist.strftime("%Y-%m-%d %H:%M:%S")

    os.makedirs(OUTPUT_DIR, exist_ok=True)
    filename = f"{IP_NAME}_TestPlan_{ts_date}_{ts_time}.xlsx"
    out_path = os.path.join(OUTPUT_DIR, filename)

    wb.save(out_path)

    # Emit GitHub Action outputs if possible
    go = os.environ.get('GITHUB_OUTPUT')
    if go:
        with open(go, 'a') as f:
            f.write(f"ist_timestamp={ist_pretty}\n")
            f.write(f"excel_path={out_path}\n")
    else:
        print(f"ist_timestamp={ist_pretty}")
        print(f"excel_path={out_path}")

if __name__ == "__main__":
    main()
