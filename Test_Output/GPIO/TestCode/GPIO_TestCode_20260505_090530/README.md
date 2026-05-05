GPIO TestCode Drop (GPIO) — Generated on 2026-05-05 09:05:30 IST

Source template: TestRepo/TemplateSrc/gpio/test_gpio_input_output_mode
IP: GPIO

Contents:
- gpio_reg_wr_rd_test/test_gpio_reg_wr_rd.c
- test_gpio_negedge_intr_en/test_gpio_negedge_intr.c
- test_gpio_pedge_all_pads_en/test_gpio_pedge_all_pads_en.c
- manifest.json
- Makefile (basic example build; adapt to your environment)

Build prerequisites:
- Ensure the template/common headers and register macro headers are available in include path:
  - test_common.h
  - test_define.h
  - test_define.c (for gpio_reg_wr_rd test as per metadata)
  - Platform headers for MIZAR_* and LSS_SYSREG_* symbols and GIC_* routines
- The tests rely on:
  - read_reg(uint32_t), write_reg(uint32_t), wait_on(uint32_t), finish(int)
  - GIC_EnableIRQ(uint32_t), GIC_ClearIRQ(uint32_t)
  - MIZAR_* register macros referenced in source

Notes:
- The implementation follows the exact procedures and acceptance criteria stated in Meta_data_sheet.
- No normalization or schema changes applied to identifiers or logic.
- Adjust USE_GPIO0 / USE_GPIO1 at compile time (e.g., -DUSE_GPIO0) to select the interrupt line as described in metadata.
