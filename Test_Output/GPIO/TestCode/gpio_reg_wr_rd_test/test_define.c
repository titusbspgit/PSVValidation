#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include <gpio/gpio_def.h>
#include <gpio/gpio_offset.h>

/* Macros from Meta (unchanged) */
#define SOFT_RST_REG_ADDRESS 0x00000000;
#define SOFT_RST_REG_DATA 0x00000000;
#define CNT 49

/*
 * Arrays generated strictly from Meta Impacted Registers using RAG mapping.
 * Order preserved exactly as provided in Meta Impacted Registers.
 * For registers without available RAG details, masks are set to 0 to exclude
 * them from RW and default validation as per Meta test logic.
 */

/* Address array (uses register macros; resolved via included gpio headers) */
const uint32_t addr_array[49] = {
    MIZAR_GPIO_GP0_GPIO_8,          /* 0  */
    MIZAR_GPIO_GP0_GPIO_9,          /* 1  */
    MIZAR_GPIO_GP0_GPIO_10,         /* 2  */
    MIZAR_GPIO_GP0_GPIO_11,         /* 3  */
    MIZAR_GPIO_GP0_GPIO_12,         /* 4  */
    MIZAR_GPIO_GP0_GPIO_13,         /* 5  */
    MIZAR_GPIO_GP0_GPIO_14,         /* 6  */
    MIZAR_GPIO_GP0_GPIO_15,         /* 7  */
    MIZAR_GPIO_GP0_GPIO_16,         /* 8  */
    MIZAR_GPIO_GP0_GPIO_17,         /* 9  */
    MIZAR_GPIO_GP0_GPIO_18,         /* 10 */
    MIZAR_GPIO_GP0_GPIO_19,         /* 11 */
    MIZAR_GPIO_GP0_GPIO_20,         /* 12 */
    MIZAR_GPIO_GP0_GPIO_21,         /* 13 */
    MIZAR_GPIO_GP0_GPIO_22,         /* 14 */
    MIZAR_GPIO_GP0_GPIO_23,         /* 15 */
    MIZAR_GPIO_GP0_GPIO_24,         /* 16 */
    MIZAR_GPIO_GP0_GPIO_25,         /* 17 */
    MIZAR_GPIO_GP0_GPIO_26,         /* 18 */
    MIZAR_GPIO_GP0_GPIO_27,         /* 19 */
    MIZAR_GPIO_GP0_GPIO_28,         /* 20 */
    MIZAR_GPIO_GP0_GPIO_29,         /* 21 */
    MIZAR_GPIO_GP0_GPIO_30,         /* 22 */
    MIZAR_GPIO_GP0_GPIO_31,         /* 23 */
    MIZAR_GPIO_GP0_GPIO_32,         /* 24 */
    MIZAR_GPIO_GP0_GPIO_33,         /* 25 */
    MIZAR_GPIO_GP0_GPIO_34,         /* 26 */
    MIZAR_GPIO_GP0_GPIO_35,         /* 27 */
    MIZAR_GPIO_GP0_GPIO_36,         /* 28 */
    MIZAR_GPIO_GP0_GPIO_37,         /* 29 */
    MIZAR_GPIO_GP0_GPIO_38,         /* 30 */
    MIZAR_GPIO_GP0_GPIO_39,         /* 31 */
    MIZAR_GPIO_GPIO_INTR_RAW_STCLR1,/* 32 */
    MIZAR_GPIO_GP0_INTR1_INTR_EN1,  /* 33 */
    MIZAR_GPIO_GP0_INTR1_INTR_STS1, /* 34 */
    MIZAR_GPIO_GP0_INTR2_INTR_EN1,  /* 35 */
    MIZAR_GPIO_GP0_INTR2_INTR_STS1, /* 36 */
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP1, /* 37 */
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP2, /* 38 */
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP3, /* 39 */
    MIZAR_GPIO_GPIO_IO_CTRL_GROUP4, /* 40 */
    MIZAR_GPIO_GPIO_DOUT_GROUP1,    /* 41 */
    MIZAR_GPIO_GPIO_DOUT_GROUP2,    /* 42 */
    MIZAR_GPIO_GPIO_DOUT_GROUP3,    /* 43 */
    MIZAR_GPIO_GPIO_DOUT_GROUP4,    /* 44 */
    MIZAR_GPIO_GPIO_DIN_GROUP1,     /* 45 */
    MIZAR_GPIO_GPIO_DIN_GROUP2,     /* 46 */
    MIZAR_GPIO_GPIO_DIN_GROUP3,     /* 47 */
    MIZAR_GPIO_GPIO_DIN_GROUP4      /* 48 */
};

/* Default/reset values array (from RAG when available; otherwise 0) */
const uint32_t default_value_array[49] = {
    0x00000000U, /* GPIO_8  (RAG) */
    0x00000000U, /* GPIO_9  (NA)  */
    0x00000000U, /* GPIO_10 (RAG) */
    0x00000000U, /* GPIO_11 (NA)  */
    0x00000000U, /* GPIO_12 (NA)  */
    0x00000000U, /* GPIO_13 (NA)  */
    0x00000000U, /* GPIO_14 (NA)  */
    0x00000000U, /* GPIO_15 (NA)  */
    0x00000000U, /* GPIO_16 (RAG) */
    0x00000000U, /* GPIO_17 (RAG) */
    0x00000000U, /* GPIO_18 (NA)  */
    0x00000000U, /* GPIO_19 (NA)  */
    0x00000000U, /* GPIO_20 (NA)  */
    0x00000000U, /* GPIO_21 (NA)  */
    0x00000000U, /* GPIO_22 (NA)  */
    0x00000000U, /* GPIO_23 (NA)  */
    0x00000000U, /* GPIO_24 (RAG) */
    0x00000000U, /* GPIO_25 (NA)  */
    0x00000000U, /* GPIO_26 (NA)  */
    0x00000000U, /* GPIO_27 (NA)  */
    0x00000000U, /* GPIO_28 (RAG) */
    0x00000000U, /* GPIO_29 (NA)  */
    0x00000000U, /* GPIO_30 (RAG) */
    0x00000000U, /* GPIO_31 (RAG) */
    0x00000000U, /* GPIO_32 (RAG) */
    0x00000000U, /* GPIO_33 (RAG) */
    0x00000000U, /* GPIO_34 (RAG) */
    0x00000000U, /* GPIO_35 (NA)  */
    0x00000000U, /* GPIO_36 (NA)  */
    0x00000000U, /* GPIO_37 (NA)  */
    0x00000000U, /* GPIO_38 (NA)  */
    0x00000000U, /* GPIO_39 (NA)  */
    0x00000000U, /* INTR_RAW_STCLR1 (NA) */
    0x00000000U, /* INTR1_EN1 (NA) */
    0x00000000U, /* INTR1_STS1 (NA) */
    0x00000000U, /* INTR2_EN1 (NA) */
    0x00000000U, /* INTR2_STS1 (NA) */
    0x00000000U, /* IO_CTRL_G1 (NA) */
    0x00000000U, /* IO_CTRL_G2 (NA) */
    0x00000000U, /* IO_CTRL_G3 (NA) */
    0x00000000U, /* IO_CTRL_G4 (NA) */
    0x00000000U, /* DOUT_G1 (NA) */
    0x00000000U, /* DOUT_G2 (NA) */
    0x00000000U, /* DOUT_G3 (NA) */
    0x00000000U, /* DOUT_G4 (NA) */
    0x00000000U, /* DIN_G1 (NA) */
    0x00000000U, /* DIN_G2 (NA) */
    0x00000000U, /* DIN_G3 (NA) */
    0x00000000U  /* DIN_G4 (NA) */
};

/* Read mask array (from RAG when available; otherwise 0) */
const uint32_t read_mask_array[49] = {
    0x00000001U, /* GPIO_8  (RAG) */
    0x00000000U, /* GPIO_9  (NA)  */
    0x00000001U, /* GPIO_10 (RAG) */
    0x00000000U, /* GPIO_11 (NA)  */
    0x00000000U, /* GPIO_12 (NA)  */
    0x00000000U, /* GPIO_13 (NA)  */
    0x00000000U, /* GPIO_14 (NA)  */
    0x00000000U, /* GPIO_15 (NA)  */
    0x00000001U, /* GPIO_16 (RAG) */
    0x00000001U, /* GPIO_17 (RAG) */
    0x00000000U, /* GPIO_18 (NA)  */
    0x00000000U, /* GPIO_19 (NA)  */
    0x00000000U, /* GPIO_20 (NA)  */
    0x00000000U, /* GPIO_21 (NA)  */
    0x00000000U, /* GPIO_22 (NA)  */
    0x00000000U, /* GPIO_23 (NA)  */
    0x00000001U, /* GPIO_24 (RAG) */
    0x00000000U, /* GPIO_25 (NA)  */
    0x00000000U, /* GPIO_26 (NA)  */
    0x00000000U, /* GPIO_27 (NA)  */
    0x00000001U, /* GPIO_28 (RAG) */
    0x00000000U, /* GPIO_29 (NA)  */
    0x00000001U, /* GPIO_30 (RAG) */
    0x00000001U, /* GPIO_31 (RAG) */
    0x00000001U, /* GPIO_32 (RAG) */
    0x00000001U, /* GPIO_33 (RAG) */
    0x00000001U, /* GPIO_34 (RAG) */
    0x00000000U, /* GPIO_35 (NA)  */
    0x00000000U, /* GPIO_36 (NA)  */
    0x00000000U, /* GPIO_37 (NA)  */
    0x00000000U, /* GPIO_38 (NA)  */
    0x00000000U, /* GPIO_39 (NA)  */
    0x00000000U, /* INTR_RAW_STCLR1 (NA) */
    0x00000000U, /* INTR1_EN1 (NA) */
    0x00000000U, /* INTR1_STS1 (NA) */
    0x00000000U, /* INTR2_EN1 (NA) */
    0x00000000U, /* INTR2_STS1 (NA) */
    0x00000000U, /* IO_CTRL_G1 (NA) */
    0x00000000U, /* IO_CTRL_G2 (NA) */
    0x00000000U, /* IO_CTRL_G3 (NA) */
    0x00000000U, /* IO_CTRL_G4 (NA) */
    0x00000000U, /* DOUT_G1 (NA) */
    0x00000000U, /* DOUT_G2 (NA) */
    0x00000000U, /* DOUT_G3 (NA) */
    0x00000000U, /* DOUT_G4 (NA) */
    0x00000000U, /* DIN_G1 (NA) */
    0x00000000U, /* DIN_G2 (NA) */
    0x00000000U, /* DIN_G3 (NA) */
    0x00000000U  /* DIN_G4 (NA) */
};

/* Write mask array (from RAG when available; otherwise 0) */
const uint32_t write_mask_array[49] = {
    0x00000000U, /* GPIO_8  (RAG) */
    0x00000000U, /* GPIO_9  (NA)  */
    0x00000000U, /* GPIO_10 (RAG) */
    0x00000000U, /* GPIO_11 (NA)  */
    0x00000000U, /* GPIO_12 (NA)  */
    0x00000000U, /* GPIO_13 (NA)  */
    0x00000000U, /* GPIO_14 (NA)  */
    0x00000000U, /* GPIO_15 (NA)  */
    0x00000000U, /* GPIO_16 (RAG) */
    0x00000000U, /* GPIO_17 (RAG) */
    0x00000000U, /* GPIO_18 (NA)  */
    0x00000000U, /* GPIO_19 (NA)  */
    0x00000000U, /* GPIO_20 (NA)  */
    0x00000000U, /* GPIO_21 (NA)  */
    0x00000000U, /* GPIO_22 (NA)  */
    0x00000000U, /* GPIO_23 (NA)  */
    0x00000000U, /* GPIO_24 (RAG) */
    0x00000000U, /* GPIO_25 (NA)  */
    0x00000000U, /* GPIO_26 (NA)  */
    0x00000000U, /* GPIO_27 (NA)  */
    0x00000000U, /* GPIO_28 (RAG) */
    0x00000000U, /* GPIO_29 (NA)  */
    0x00000000U, /* GPIO_30 (RAG) */
    0x00000000U, /* GPIO_31 (RAG) */
    0x00000000U, /* GPIO_32 (RAG) */
    0x00000000U, /* GPIO_33 (RAG) */
    0x00000000U, /* GPIO_34 (RAG) */
    0x00000000U, /* GPIO_35 (NA)  */
    0x00000000U, /* GPIO_36 (NA)  */
    0x00000000U, /* GPIO_37 (NA)  */
    0x00000000U, /* GPIO_38 (NA)  */
    0x00000000U, /* GPIO_39 (NA)  */
    0x00000000U, /* INTR_RAW_STCLR1 (NA) */
    0x00000000U, /* INTR1_EN1 (NA) */
    0x00000000U, /* INTR1_STS1 (NA) */
    0x00000000U, /* INTR2_EN1 (NA) */
    0x00000000U, /* INTR2_STS1 (NA) */
    0x00000000U, /* IO_CTRL_G1 (NA) */
    0x00000000U, /* IO_CTRL_G2 (NA) */
    0x00000000U, /* IO_CTRL_G3 (NA) */
    0x00000000U, /* IO_CTRL_G4 (NA) */
    0x00000000U, /* DOUT_G1 (NA) */
    0x00000000U, /* DOUT_G2 (NA) */
    0x00000000U, /* DOUT_G3 (NA) */
    0x00000000U, /* DOUT_G4 (NA) */
    0x00000000U, /* DIN_G1 (NA) */
    0x00000000U, /* DIN_G2 (NA) */
    0x00000000U, /* DIN_G3 (NA) */
    0x00000000U  /* DIN_G4 (NA) */
};

/* Skip arrays from Meta (unchanged) */
const uint32_t skip_array[49] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,1,1,1,1,1,1,1,1,0,0,0,0 };
const uint32_t skip_rst_array[49] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1 };
