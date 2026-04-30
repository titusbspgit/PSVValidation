GPIO Test Code Generation (IST timestamped)

Source references:
- TestPlan Excel: 1777515896761/GPIO_TestPlan_20260429_215621.xlsx
- Approved spec headers: <gpio/gpio_def.h>, <gpio/gpio_offset.h>
- Template (structure reference only): TestRepo/TemplateSrc/gpio/test_gpio_input_output_mode

Generated at (IST): 2026-04-30 12:00:00

Generated tests:
1) gpio_reg_wr_rd_test
   - Implements: reset-default checks (LSB cleared on read), masked write/read for six patterns
2) test_gpio_negedge_intr_en
   - Implements: per-pin negedge enable via per-pin write masks, edge generation via PAD_DRIVER_ADDR, bounded wait, group status verify/clear
3) test_gpio_pedge_all_pads_en
   - Implements: pedge enable all pads, group IO input mode via IO_CTRL_GROUP1..4, bounded wait, group status verify/clear

Notes/Assumptions:
- Program files include only test_define.c; test_define.c contains provided headers/defines/arrays AS-IS.
- PAD_DRIVER_ADDR (0xA0243ffc) is used as explicitly authorized for interrupt edge generation.
- Where exact field-level enables/clears are not enumerated, code writes the per-register write_mask to assert/clear writable fields deterministically.
- Pass/Fail strictly via finish(0/1). DEBUG_DISPLAY guards optional logging.
