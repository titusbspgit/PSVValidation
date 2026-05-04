# test_gpio_negedge_intr_en

Objective:
Enable and validate negative-edge interrupts for GPIO pads 8–39. For each pad, generate a falling edge via external pad control (0xA0243ffc), wait for interrupt service, verify DIN low, confirm raw/group status set and proper clear, and ensure platform/system interrupt clear.

Acceptance Criteria:
1) ISR is observed before timeout for each pad.
2) DIN reads low after the falling edge.
3) Raw and group interrupt status bits assert and are cleared by the handler.
4) System raw status clear succeeds.
