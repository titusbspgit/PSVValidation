#ifndef TEST_DEFINE_C_FILE
#define TEST_DEFINE_C_FILE

/* Meta Headers (included unchanged as provided) */
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "test_define.c"
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

/* Meta Macros (included unchanged as provided) */
#define CNT 49; 
#define SOFT_RST_REG_ADDRESS 0x00000000; 
#define SOFT_RST_REG_DATA 0x00000000

/*
 * Impacted Registers (from Meta Impacted Registers):
 * MIZAR_GPIO_GP0_GPIO_8, MIZAR_GPIO_GP0_GPIO_9, MIZAR_GPIO_GP0_GPIO_10, MIZAR_GPIO_GP0_GPIO_11,
 * MIZAR_GPIO_GP0_GPIO_12, MIZAR_GPIO_GP0_GPIO_13, MIZAR_GPIO_GP0_GPIO_14, MIZAR_GPIO_GP0_GPIO_15,
 * MIZAR_GPIO_GP0_GPIO_16, MIZAR_GPIO_GP0_GPIO_17, MIZAR_GPIO_GP0_GPIO_18, MIZAR_GPIO_GP0_GPIO_19,
 * MIZAR_GPIO_GP0_GPIO_20, MIZAR_GPIO_GP0_GPIO_21, MIZAR_GPIO_GP0_GPIO_22, MIZAR_GPIO_GP0_GPIO_23,
 * MIZAR_GPIO_GP0_GPIO_24, MIZAR_GPIO_GP0_GPIO_25, MIZAR_GPIO_GP0_GPIO_26, MIZAR_GPIO_GP0_GPIO_27
 */

/*
 * Arrays generated using RAG mapping where available.
 * For entries with NA in RAG, values are set to 0 to avoid speculative behavior.
 * This ensures code only operates on registers with defined masks.
 */
static const unsigned int addr_array[20] = {
    MIZAR_GPIO_GP0_GPIO_8,
    MIZAR_GPIO_GP0_GPIO_9,
    MIZAR_GPIO_GP0_GPIO_10,
    MIZAR_GPIO_GP0_GPIO_11,
    MIZAR_GPIO_GP0_GPIO_12,
    MIZAR_GPIO_GP0_GPIO_13,
    MIZAR_GPIO_GP0_GPIO_14,
    MIZAR_GPIO_GP0_GPIO_15,
    MIZAR_GPIO_GP0_GPIO_16,
    MIZAR_GPIO_GP0_GPIO_17,
    MIZAR_GPIO_GP0_GPIO_18,
    MIZAR_GPIO_GP0_GPIO_19,
    MIZAR_GPIO_GP0_GPIO_20,
    MIZAR_GPIO_GP0_GPIO_21,
    MIZAR_GPIO_GP0_GPIO_22,
    MIZAR_GPIO_GP0_GPIO_23,
    MIZAR_GPIO_GP0_GPIO_24,
    MIZAR_GPIO_GP0_GPIO_25,
    MIZAR_GPIO_GP0_GPIO_26,
    MIZAR_GPIO_GP0_GPIO_27
};

/* Default/reset values from RAG when available; otherwise 0 */
static const unsigned int default_value_array[20] = {
    0x00000000, /* GP0_GPIO_8: documented data_in[0] reset=0 */
    0x00000000, /* GP0_GPIO_9: NA */
    0x00000000, /* GP0_GPIO_10: NA */
    0x00000000, /* GP0_GPIO_11: NA */
    0x00000000, /* GP0_GPIO_12: NA */
    0x00000000, /* GP0_GPIO_13: NA */
    0x00000000, /* GP0_GPIO_14: NA */
    0x00000000, /* GP0_GPIO_15: NA */
    0x00000000, /* GP0_GPIO_16: documented data_in[0] reset=0 */
    0x00000000, /* GP0_GPIO_17: NA */
    0x00000000, /* GP0_GPIO_18: NA */
    0x00000000, /* GP0_GPIO_19: NA */
    0x00000000, /* GP0_GPIO_20: NA */
    0x00000000, /* GP0_GPIO_21: NA */
    0x00000000, /* GP0_GPIO_22: NA */
    0x00000000, /* GP0_GPIO_23: NA */
    0x00000000, /* GP0_GPIO_24: NA */
    0x00000000, /* GP0_GPIO_25: NA */
    0x00000000, /* GP0_GPIO_26: NA */
    0x00000000  /* GP0_GPIO_27: NA */
};

/* Readable bit masks from RAG when available; otherwise 0 (not readable/used) */
static const unsigned int read_mask_array[20] = {
    0x00000001, /* GP0_GPIO_8: readable mask */
    0x00000000, /* GP0_GPIO_9: NA */
    0x00000000, /* GP0_GPIO_10: NA */
    0x00000000, /* GP0_GPIO_11: NA */
    0x00000000, /* GP0_GPIO_12: NA */
    0x00000000, /* GP0_GPIO_13: NA */
    0x00000000, /* GP0_GPIO_14: NA */
    0x00000000, /* GP0_GPIO_15: NA */
    0x00000001, /* GP0_GPIO_16: readable mask */
    0x00000000, /* GP0_GPIO_17: NA */
    0x00000000, /* GP0_GPIO_18: NA */
    0x00000000, /* GP0_GPIO_19: NA */
    0x00000000, /* GP0_GPIO_20: NA */
    0x00000000, /* GP0_GPIO_21: NA */
    0x00000000, /* GP0_GPIO_22: NA */
    0x00000000, /* GP0_GPIO_23: NA */
    0x00000000, /* GP0_GPIO_24: NA */
    0x00000000, /* GP0_GPIO_25: NA */
    0x00000000, /* GP0_GPIO_26: NA */
    0x00000000  /* GP0_GPIO_27: NA */
};

/* Writable bit masks from RAG when available; otherwise 0 (write not allowed) */
static const unsigned int write_mask_array[20] = {
    0x00000000, /* GP0_GPIO_8: not writable */
    0x00000000, /* GP0_GPIO_9: NA */
    0x00000000, /* GP0_GPIO_10: NA */
    0x00000000, /* GP0_GPIO_11: NA */
    0x00000000, /* GP0_GPIO_12: NA */
    0x00000000, /* GP0_GPIO_13: NA */
    0x00000000, /* GP0_GPIO_14: NA */
    0x00000000, /* GP0_GPIO_15: NA */
    0x00000000, /* GP0_GPIO_16: not writable */
    0x00000000, /* GP0_GPIO_17: NA */
    0x00000000, /* GP0_GPIO_18: NA */
    0x00000000, /* GP0_GPIO_19: NA */
    0x00000000, /* GP0_GPIO_20: NA */
    0x00000000, /* GP0_GPIO_21: NA */
    0x00000000, /* GP0_GPIO_22: NA */
    0x00000000, /* GP0_GPIO_23: NA */
    0x00000000, /* GP0_GPIO_24: NA */
    0x00000000, /* GP0_GPIO_25: NA */
    0x00000000, /* GP0_GPIO_26: NA */
    0x00000000  /* GP0_GPIO_27: NA */
};

/* Skip arrays (extracted from Meta Arrays) */
static const unsigned int skip_array[20] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

static const unsigned int skip_rst_array[20] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};

#endif /* TEST_DEFINE_C_FILE */
