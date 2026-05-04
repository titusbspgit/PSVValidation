# Simple build script (adjust to your environment)
TARGET ?= test_gpio_negedge_intr_en
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
