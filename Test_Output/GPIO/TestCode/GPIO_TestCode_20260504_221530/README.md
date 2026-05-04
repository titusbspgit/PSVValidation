GPIO Negative-Edge Interrupt Enable Test (test_gpio_negedge_intr_en)

Purpose
- Validates negative-edge interrupt behavior on GPIO pins 8..39:
  - Per-pin configuration for input mode and negative-edge detection
  - Falling-edge generation via external pad driver register 0xA0243ffc
  - Interrupt service path, status validation, and clear sequence

Test source
- src/test_gpio_negedge_intr_en.c

Inputs and selection
- Compile-time selection (choose exactly one; defaults to GPIO0 if none given):
  - USE_GPIO0 => GIC IRQ 87, system bit LSS_SYSREG_*_GPIO0_INTR
  - USE_GPIO1 => GIC IRQ 88, system bit LSS_SYSREG_*_GPIO1_INTR

Build
- Using Make:
  - make USE_GPIO0=1
  - or make USE_GPIO1=1
- Using CMake:
  - cmake -S . -B build -DUSE_GPIO0=ON
  - cmake --build build

Run
- Ensure vector table routes the platform’s default IRQ to Default_IRQHandler
- Program flow:
  1) Enables system interrupt (MIZAR_LSS_SYSREG_INTR_EN1)
  2) Unmasks GIC line 87/88
  3) Sets all pins [8..39] to input, enables negedge detect, clears per-pin raw
  4) For each pin i=0..31:
     - Clears group raw for that bit
     - Enables only that bit in MIZAR_GPIO_GP0_INTR1_INTR_EN1
     - Generates a falling edge via 0xA0243ffc by driving ~ (1<<i)
     - Waits (timeout=5000) for ISR to run
  5) ISR:
     - Restores pad high
     - Verifies DIN low on the pin (bit0==0)
     - Confirms group status has the tested bit set
     - Clears per-pin and group raw, confirms group status clears to 0x0
     - Clears system raw and GIC line

Expected results
- Pass if finish(test_err) is called with 0
- A timeout, missing DIN low, missing group status set/clear, or uncleared system raw increments test_err

Impacted registers (from Meta_data_sheet)
- MIZAR_GPIO_GP0_GPIO_8
- MIZAR_GPIO_GP0_INTR1_INTR_EN1
- MIZAR_GPIO_GP0_INTR1_INTR_STS1
- MIZAR_GPIO_GPIO_INTR_RAW_STCLR1
- MIZAR_LSS_SYSREG_INTR_EN1
- MIZAR_LSS_SYSREG_RAW_STCR1

Edge generator
- External pad driver register: 0xA0243ffc
