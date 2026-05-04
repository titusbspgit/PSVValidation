# test_gpio_pedge_all_pads_en

Objective:
Enable positive-edge interrupts on GPIO pads 8–39 with group status handling. ISR masks group, clears per-pin raw for all pads asserted, verifies group clear, clears system raw status, re-enables group, and clears platform IRQ.

Acceptance Criteria:
1) ISR occurs before timeout on each pad.
2) Group raw status is non-zero on entry to ISR and zero after clearing.
3) System raw status bit is cleared.
4) Group re-enabled after service.
