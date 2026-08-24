/*
 // Author - AI Force 1.3.2. Date 25-06-2025
 // (EMBENGG-SYSAPPS)
*/

/*
 * Test Case Name : pcie_reg_wr_rd_test
 * Purpose        : Definitions, macros, register address arrays, default
 *                  value arrays, write mask arrays, and test pattern arrays
 *                  for PCIe register reset-value verification and
 *                  write-read-back validation across DBI DSP controller,
 *                  SII interface, and PHY register groups.
 */

/* ---------------------------------------------------------------- */
/* Debug logging support                                            */
/* ---------------------------------------------------------------- */
#ifdef DEBUG_DISPLAY
#include <stdio.h>
#define debug_print(...) printf(__VA_ARGS__)
#else
#define debug_print(...) do {} while(0)
#endif

#ifndef LOGI
#define LOGI(...) printf(__VA_ARGS__)
#endif

/* ---------------------------------------------------------------- */
/* Array Size Constants                                             */
/* ---------------------------------------------------------------- */
#define CTL_REG_COUNT                                       5
#define SII_REG_COUNT                                       3
#define PHY_REG_COUNT                                       3
#define CHK_PATTERN_COUNT                                   3
#define CHK_VAL_TOTAL                                       6
#define CHK_VAL_PHY_COUNT                                   3

/* ---------------------------------------------------------------- */
/* PHY 16-bit Extraction Mask                                       */
/* ---------------------------------------------------------------- */
#define PHY_16BIT_MASK                                      0x0000FFFFU
#define PHY_WRITE_MASK_COMMON                               0x00001FFFU
#define PHY_RST_CONTROL_VALUE                               0x01203000U

/* ---------------------------------------------------------------- */
/* PCIE0 DBI DSP Controller Register Addresses                     */
/*   MSI_CAP_OFF_08H_REG, MSI_CAP_OFF_10H_REG,                    */
/*   FILTER_MASK_2_OFF, AXI_MSTR_MSG_ADDR_HIGH_OFF, UTILITY_OFF   */
/* ---------------------------------------------------------------- */
#ifndef mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG
extern volatile unsigned int mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG;
#endif
#ifndef mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG
extern volatile unsigned int mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG;
#endif
#ifndef mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF
extern volatile unsigned int mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF;
#endif
#ifndef mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF
extern volatile unsigned int mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF;
#endif
#ifndef mizar_PCIE0_DBI_DSP_UTILITY_OFF
extern volatile unsigned int mizar_PCIE0_DBI_DSP_UTILITY_OFF;
#endif

/* ---------------------------------------------------------------- */
/* PCIE1 DBI DSP Controller Register Addresses                     */
/* ---------------------------------------------------------------- */
#ifndef mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG
extern volatile unsigned int mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG;
#endif
#ifndef mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG
extern volatile unsigned int mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG;
#endif
#ifndef mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF
extern volatile unsigned int mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF;
#endif
#ifndef mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF
extern volatile unsigned int mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF;
#endif
#ifndef mizar_PCIE1_DBI_DSP_UTILITY_OFF
extern volatile unsigned int mizar_PCIE1_DBI_DSP_UTILITY_OFF;
#endif

/* ---------------------------------------------------------------- */
/* PCIE0 SII Interface Register Addresses                          */
/* ---------------------------------------------------------------- */
#ifndef mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2
extern volatile unsigned int mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2;
#endif
#ifndef mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3
extern volatile unsigned int mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3;
#endif
#ifndef mizar_PCIE0_SII_PHY_CONTROL_23
extern volatile unsigned int mizar_PCIE0_SII_PHY_CONTROL_23;
#endif

/* ---------------------------------------------------------------- */
/* PCIE1 SII Interface Register Addresses                          */
/* ---------------------------------------------------------------- */
#ifndef mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2
extern volatile unsigned int mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2;
#endif
#ifndef mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3
extern volatile unsigned int mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3;
#endif
#ifndef mizar_PCIE1_SII_PHY_CONTROL_23
extern volatile unsigned int mizar_PCIE1_SII_PHY_CONTROL_23;
#endif

/* ---------------------------------------------------------------- */
/* PHY Reset Control Register Addresses                            */
/* ---------------------------------------------------------------- */
#ifndef mizar_PCIE0_SII_PHY_RST_CONTROL
extern volatile unsigned int mizar_PCIE0_SII_PHY_RST_CONTROL;
#endif
#ifndef mizar_PCIE1_SII_PHY_RST_CONTROL
extern volatile unsigned int mizar_PCIE1_SII_PHY_RST_CONTROL;
#endif

/* ---------------------------------------------------------------- */
/* FV Framework Type Declarations                                   */
/* ---------------------------------------------------------------- */
#ifndef TESTS_ITEM_DEFINED
#define TESTS_ITEM_DEFINED
typedef struct {
    const char *test_name;
} TestsItem;
#endif

#ifndef TEST_OUTPUT_DEFINED
#define TEST_OUTPUT_DEFINED
typedef struct {
    int status;
} TestOutput;
#endif

/* ---------------------------------------------------------------- */
/* Register Address Arrays                                         */
/* ---------------------------------------------------------------- */

/* Step 1: PCIE0 DBI DSP controller register addresses */
unsigned int rc0_ctl_addr[CTL_REG_COUNT] = {
    (unsigned int)&mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_08H_REG,
    (unsigned int)&mizar_PCIE0_DBI_DSP_MSI_CAP_OFF_10H_REG,
    (unsigned int)&mizar_PCIE0_DBI_DSP_FILTER_MASK_2_OFF,
    (unsigned int)&mizar_PCIE0_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
    (unsigned int)&mizar_PCIE0_DBI_DSP_UTILITY_OFF
};

/* Step 2: PCIE1 DBI DSP controller register addresses */
unsigned int rc1_ctl_addr[CTL_REG_COUNT] = {
    (unsigned int)&mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_08H_REG,
    (unsigned int)&mizar_PCIE1_DBI_DSP_MSI_CAP_OFF_10H_REG,
    (unsigned int)&mizar_PCIE1_DBI_DSP_FILTER_MASK_2_OFF,
    (unsigned int)&mizar_PCIE1_DBI_DSP_AXI_MSTR_MSG_ADDR_HIGH_OFF,
    (unsigned int)&mizar_PCIE1_DBI_DSP_UTILITY_OFF
};

/* Step 3: PCIE0 SII interface register addresses */
unsigned int sii0_addr[SII_REG_COUNT] = {
    (unsigned int)&mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER2,
    (unsigned int)&mizar_PCIE0_SII_PCIE0_TRANSMIT_HEADER3,
    (unsigned int)&mizar_PCIE0_SII_PHY_CONTROL_23
};

/* Step 4: PCIE1 SII interface register addresses */
unsigned int sii1_addr[SII_REG_COUNT] = {
    (unsigned int)&mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER2,
    (unsigned int)&mizar_PCIE1_SII_PCIE1_TRANSMIT_HEADER3,
    (unsigned int)&mizar_PCIE1_SII_PHY_CONTROL_23
};

/* Step 5: PCIE0 PHY register addresses */
unsigned int phy0_addr[PHY_REG_COUNT] = {
    0xE68860B8U,
    0xE68862B8U,
    0xE68864B8U
};

/* Step 5: PCIE1 PHY register addresses */
unsigned int phy1_addr[PHY_REG_COUNT] = {
    0xE68A60B8U,
    0xE68A62B8U,
    0xE68A64B8U
};

/* ---------------------------------------------------------------- */
/* Default Value Arrays                                             */
/* ---------------------------------------------------------------- */

/* Step 6: Controller register default values (all 0x0) */
unsigned int ctl_default[CTL_REG_COUNT] = {
    0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U, 0x00000000U
};

/* Step 6: SII register default values (all 0x0) */
unsigned int sii_default[SII_REG_COUNT] = {
    0x00000000U, 0x00000000U, 0x00000000U
};

/* Step 6: PHY0 register default values (all 0x0) */
unsigned int phy0_default[PHY_REG_COUNT] = {
    0x00000000U, 0x00000000U, 0x00000000U
};

/* Step 6: PHY1 register default values (all 0x0) */
unsigned int phy1_default[PHY_REG_COUNT] = {
    0x00000000U, 0x00000000U, 0x00000000U
};

/* ---------------------------------------------------------------- */
/* Write Mask Arrays                                                */
/* ---------------------------------------------------------------- */

/* Step 7: SII0 write masks */
unsigned int sii0_write_mask[SII_REG_COUNT] = {
    0xFFFFFFFFU, 0xFFFFFFFFU, 0x000F000FU
};

/* Step 7: SII1 write masks */
unsigned int sii1_write_mask[SII_REG_COUNT] = {
    0xFFFFFFFFU, 0xFFFFFFFFU, 0x000F000FU
};

/* Step 7: PHY0 write masks */
unsigned int phy0_write_mask[PHY_REG_COUNT] = {
    0x00001FFFU, 0x00001FFFU, 0x00001FFFU
};

/* Step 7: PHY1 write masks */
unsigned int phy1_write_mask[PHY_REG_COUNT] = {
    0x00001FFFU, 0x00001FFFU, 0x00001FFFU
};

/* ---------------------------------------------------------------- */
/* Test Pattern Arrays                                              */
/* ---------------------------------------------------------------- */

/* Step 17: Check value patterns (6 total, first 3 used in loop) */
unsigned int chk_val[CHK_VAL_TOTAL] = {
    0xFFFFFFFFU, 0xAAAAAAAAU, 0x55555555U,
    0x00000000U, 0xA5A5A5A5U, 0xFFFF0000U
};

/* Step 17: PHY-specific check value patterns */
unsigned int chk_val_phy[CHK_VAL_PHY_COUNT] = {
    0x00007BAFU, 0x00000001U, 0x0000003BU
};

/* ---------------------------------------------------------------- */
/* Extern Function Declarations                                     */
/* ---------------------------------------------------------------- */
extern void finish(int status);
extern unsigned int read_reg(unsigned int addr);
extern void write_reg(unsigned int addr, unsigned int value);

/* ---------------------------------------------------------------- */
/* Macros - No additional macros were supplied in the input.        */
/* ---------------------------------------------------------------- */
