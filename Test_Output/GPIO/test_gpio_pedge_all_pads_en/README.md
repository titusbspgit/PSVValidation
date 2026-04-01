# test_gpio_pedge_all_pads_en

Generation timestamp: 2026-04-01T07:20:45Z
Author: Test_Stage3 Agent

Summary
- Purpose: Enable posedge interrupts on GPIO[8..39], generate a rising edge per pin, verify ISR servicing and group/system status clear, then re-enable for next iteration.
- Entry function: void test_case(void)
- Files:
  - program.c (includes only "test_define.c")
  - test_define.c (all headers/defines placed at top)

Includes (from testcase context)
- #include <lss_sysreg.h>
- #include <stdio.h>
- #include <test_define.c>
- #include <test_common.h>
- #include <gpio/gpio_def.h>
- #include <gpio/gpio_offset.h>

Defines (from testcase context)
- #define CNT 49

Implemented flow (per CSV Test Steps / Procedure)
1) Enable IRQ: GPIO0 -> 87 or GPIO1 -> 88 via GIC_EnableIRQ().
2) Enable system interrupt: write_reg(MIZAR_LSS_SYSREG_INTR_EN1, LSS_SYSREG_INTR_EN1_GPIOx_INTR).
3) Configure per-pin posedge on GPIO[8..39]: for i=0..31 write 0x00020000 to (MIZAR_GPIO_GP0_GPIO_8 + i*4).
4) Set IO control groups 1..4 to input mode: write 0x000000FF to MIZAR_GPIO_GPIO_IO_CTRL_GROUP1..4.
5) Enable group interrupt: write 0xFFFFFFFF to MIZAR_GPIO_GP0_INTR1_INTR_EN1.
6) For each pin i in [0..31]:
   - Drive low: write 0x00000000 to 0xA0243ffc, wait.
   - Arm ISR: int_pend = 1; drive high then wr_val = (1<<i) to 0xA0243ffc to create rising edge.
   - Poll until int_pend == 0 with timeout; on timeout, increment error.
   - Drive low again before next iteration.
7) Call finish(0) on success else finish(1).

ISR behavior (Default_IRQHandler)
- On entry: int_pend = 0.
- Read MIZAR_GPIO_GP0_INTR1_INTR_STS1 and require non-zero.
- Temporarily mask group enable to 0.
- Clear per-pin RAW: for j=0..31 write 0x00010000 to (MIZAR_GPIO_GP0_GPIO_8 + j*4), then verify group status becomes 0.
- Clear system RAW via MIZAR_LSS_SYSREG_RAW_STCR1 using GPIO0/GPIO1 bit.
- Re-enable group and clear GIC IRQ (87/88).

Acceptance criteria mapping
- No per-iteration timeout during polling.
- ISR sees non-zero group status, then group status becomes 0 after clears.
- System RAW_STCR1 bit cleared for selected GPIOx.
- Final finish(0) if test_err == 0; otherwise finish(1).

TODOs / Placeholders
- GPIO0/GPIO1 selection: expected via build-time defines; warnings emitted if not defined.
- Impacted_Register_Spec_RAG: No data available for this testcase at generation time; code relies on header-provided symbols only and literal address 0xA0243ffc from CSV.
- No register field definitions are invented; test uses write_reg/read_reg with known symbols and constants per CSV.
