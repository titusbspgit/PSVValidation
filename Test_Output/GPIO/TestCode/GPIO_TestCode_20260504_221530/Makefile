# Makefile for GPIO negedge interrupt enable test
# Base includes and flags may be provided by the template we copied.
PROJECT_NAME := test_gpio_negedge_intr_en

SRCS ?=
SRCS += src/test_gpio_negedge_intr_en.c

INCLUDES ?=
CFLAGS ?= -O2 -Wall -Wextra
LDFLAGS ?=

# Allow selection of GPIO instance via make USE_GPIO0=1 or USE_GPIO1=1
ifeq ($(USE_GPIO0),1)
CFLAGS += -DUSE_GPIO0
endif
ifeq ($(USE_GPIO1),1)
CFLAGS += -DUSE_GPIO1
endif

# If the template provides common.mk, include it; else fall back to a simple build.
ifneq ("$(wildcard common.mk)","")
include common.mk
else

# Toolchain (override as per your environment)
CC      ?= gcc
AR      ?= ar
OBJS    := $(SRCS:.c=.o)
OUT     := $(PROJECT_NAME).elf

all: $(OUT)

$(OUT): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -f $(OBJS) $(OUT)

.PHONY: all clean
endif
