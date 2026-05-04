GPIO TestCode (Generated)

Timestamp (IST): 2026-05-04 19:15:53
IP: GPIO
Testcases:
- gpio_reg_wr_rd_test
- test_gpio_negedge_intr_en
- test_gpio_pedge_all_pads_en

Build notes:
- These tests rely on platform HAL providing: read_reg, write_reg, wait_on, finish, GIC_EnableIRQ, GIC_ClearIRQ, Default_IRQHandler linkage.
- Headers expected from platform toolchain: test_common.h, lss_sysreg.h, gpio/gpio_def.h, gpio/gpio_offset.h, and LSS sysreg macros (e.g., LSS_SYSREG_INTR_EN1_GPIO0_INTR).
- Define exactly one of: GPIO0 or GPIO1 at compile time to select the target GPIO instance and IRQ (87 for GPIO0, 88 for GPIO1).
- Optional define: DEBUG_DISPLAY for verbose logs.
- The code style and include usage follow the template at TestRepo/TemplateSrc/gpio/test_gpio_input_output_mode.

Generic build (example only; adjust CC/INCLUDES to your environment):
- cd into a testcase directory and run: make INCLUDES="-I. -I<path-to-platform-includes>" CC=<your-cc>

Directory contents:
- Each testcase folder contains: program.c (test logic), optional test_define.c (arrays/masks), and a simple Makefile.
