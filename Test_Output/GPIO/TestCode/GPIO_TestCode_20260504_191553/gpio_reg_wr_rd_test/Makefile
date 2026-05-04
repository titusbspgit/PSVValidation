# Simple build script (adjust to your environment)
TARGET ?= gpio_reg_wr_rd_test
SRCS := program.c
OBJS := $(SRCS:.c=.o)
CC ?= gcc
CFLAGS ?= -O2 -Wall
INCLUDES ?= -I.

all: $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS)
