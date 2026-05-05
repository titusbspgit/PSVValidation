# Basic example Makefile. Adjust toolchain, includes, and libs per your environment.

TARGETS := test_gpio_reg_wr_rd test_gpio_negedge_intr test_gpio_pedge_all_pads_en

CFLAGS ?= -O2 -Wall -Wextra -std=c11
# Add template include paths here:
# INCLUDES := -I./TestRepo/TemplateSrc/gpio/test_gpio_input_output_mode -I./include
INCLUDES :=

LDFLAGS ?=
LDLIBS ?=

all: $(TARGETS)

test_gpio_reg_wr_rd: gpio_reg_wr_rd_test/test_gpio_reg_wr_rd.c
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $< $(LDFLAGS) $(LDLIBS)

test_gpio_negedge_intr: test_gpio_negedge_intr_en/test_gpio_negedge_intr.c
	$(CC) $(CFLAGS) $(INCLUDES) -DUSE_GPIO0 -o $@ $< $(LDFLAGS) $(LDLIBS)

test_gpio_pedge_all_pads_en: test_gpio_pedge_all_pads_en/test_gpio_pedge_all_pads_en.c
	$(CC) $(CFLAGS) $(INCLUDES) -DUSE_GPIO0 -o $@ $< $(LDFLAGS) $(LDLIBS)

clean:
	rm -f $(TARGETS)
