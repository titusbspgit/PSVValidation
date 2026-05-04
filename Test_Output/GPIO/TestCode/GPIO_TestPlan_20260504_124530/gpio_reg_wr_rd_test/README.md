# gpio_reg_wr_rd_test

Objective:
Validate GPIO register default values and masked write/read behavior across key GPIO registers. Skip registers as dictated by skip_array.

Acceptance Criteria:
- Default value checks: (read_reg(addr) & read_mask) == (default_value & read_mask) for all non-skipped entries.
- Masked write/read: write patterns constrained by write_mask and verify readback under read_mask.
- Overall PASS when no mismatches are recorded.

References:
- Impacted registers, masks, defaults sourced from gpio_def.h/gpio_offset.h via test_define.c arrays.
- See program.c for detailed flow.
