# GPIO Test Plan Output (Generated)

Timestamp (IST): 2026-05-04 12:45:30
IP: GPIO
Source Template: TestRepo/TemplateSrc/gpio/test_gpio_input_output_mode

Generated testcases (scaffolded under this directory):
- gpio_reg_wr_rd_test
- test_gpio_negedge_intr_en
- test_gpio_pedge_all_pads_en

Build instructions (per testcase):
- cd into the testcase directory and run `make` (requires project VERIF environment and toolchain as used by existing GPIO tests)
- Artifacts and build variables come from $(WORKINGDIR)/verif/C_ENV/SOC_ENV/make.include.c_env

Dependencies and headers:
- Uses platform headers: lss_sysreg.h, test_common.h
- Uses GPIO headers: gpio/gpio_def.h, gpio/gpio_offset.h
- These are expected to be available via the existing build environment (VERIF path). The Makefiles add `-Iinclude` for local includes.

Notes:
- program.c includes test_define.c via preprocessor include to align with template convention.
- Refer to each testcase README for objectives, steps, and pass criteria, derived from the provided test plan metadata.
