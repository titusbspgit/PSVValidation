# GPIO Testcases (Auto-Generated)

This directory contains auto-generated deterministic embedded C testcases for the GPIO IP, produced by Ag-Emb-Mpsoc-Stage3 Agent.

Included tests:
- gpio_reg_wr_rd_test.c
- test_gpio_pedge_all_pads_en.c

Common build assumptions:
- Project provides test framework APIs: read_reg, write_reg, wait_on, finish, enable_irq, clear_irq.
- Required headers exist in repository include paths: test_common.h, test_define.c, lss_sysreg.h, gpio/gpio_def.h, gpio/gpio_offset.h.

---

## gpio_reg_wr_rd_test.c

Description:
- Default value verification for impacted registers using (read & 0xFFFFFFFE) == default_value and skip rules.
- Masked write/read verification across 6 patterns with read/write masks and skip arrays.

Acceptance criteria:
- Pass when def_fail_cnt == 0 and wr_fail_cnt == 0; else Fail.

---

## test_gpio_pedge_all_pads_en.c

Description:
- Configure GPIO[8..39] for input mode and enable positive-edge interrupts (PEIE bit17).
- Enable group and system interrupts, external stimulus must generate rising edges.
- ISR masks group, validates non-zero status, clears per-pin RAW (bit16), verifies group/system clears, and re-enables group.

Acceptance criteria:
- No timeouts waiting for ISR; group status non-zero during service and zero after clear; system status clears; overall test_err == 0.

Run notes:
- For GPIO0 path, GIC IRQ 87 and LSS system bits LSS_SYSREG_INTR_EN1_GPIO0_INTR / LSS_SYSREG_RAW_STCR1_GPIO0_INTR are used.
- Define USE_GPIO1 at compile time to use GPIO1 path (GIC IRQ 88 and corresponding LSS bits).

---

Generated: IST 2026-05-04 22:30:00
