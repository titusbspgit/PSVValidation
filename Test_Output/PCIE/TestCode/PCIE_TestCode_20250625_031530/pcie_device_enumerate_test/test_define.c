/*
 // Author - AI Force 1.3.2. Date 25-06-2025
 // (EMBENGG-SYSAPPS)
*/

/*
 * Test Case Name : pcie_device_enumerate_test
 * Purpose        : Definitions, macros, and register addresses for PCIe
 *                  device enumeration testcase. Covers COHERENCY_CONTROL_3_OFF,
 *                  SII link status, TYPE1_DEV_ID_VEND_ID_REG,
 *                  TYPE1_STATUS_COMMAND_REG, and BAR register offsets.
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
/* Control / Synchronization Register                               */
/* ---------------------------------------------------------------- */
#define CTRL_REG_ADDR                                       0xE6004100U
#define SYNC_HANDSHAKE_VALUE                                0x12345678U

/* ---------------------------------------------------------------- */
/* COHERENCY_CONTROL_3_OFF Registers (PCIE0 and PCIE1)             */
/* ---------------------------------------------------------------- */
#ifndef mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF
extern volatile unsigned int mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF;
#endif

#ifndef mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF
extern volatile unsigned int mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF;
#endif

/* ---------------------------------------------------------------- */
/* SII Link Status Registers                                       */
/* ---------------------------------------------------------------- */
#define SII_LINK_STATUS_OFFSET                              0xC0U
#define SII_LINK_UP_MASK                                    0xD1U

#ifndef SII0_LINK_STATUS_REG
extern volatile unsigned int SII0_LINK_STATUS_REG;
#endif

#ifndef SII1_LINK_STATUS_REG
extern volatile unsigned int SII1_LINK_STATUS_REG;
#endif

/* ---------------------------------------------------------------- */
/* PCIe Slave Port Register Offsets                                 */
/*   TYPE1_DEV_ID_VEND_ID_REG  -> offset 0x0                       */
/*   TYPE1_STATUS_COMMAND_REG  -> offset 0x4                        */
/* ---------------------------------------------------------------- */
#define PCIE_SLV_VENDOR_ID_OFFSET                           0x00U
#define PCIE_SLV_CMD_STATUS_OFFSET                          0x04U
#define PCIE_CMD_MEM_IO_BUSMASTER                           0x7U

/* ---------------------------------------------------------------- */
/* BAR Register Offsets                                             */
/*   BAR0_REG                    -> offset 0x10                     */
/*   BAR1_REG                    -> offset 0x14                     */
/*   SEC_LAT_TIMER_SUB_BUS_...  -> offset 0x18                     */
/*   SEC_STAT_IO_LIMIT_...      -> offset 0x1C                     */
/*   MEM_LIMIT_MEM_BASE_REG     -> offset 0x20                     */
/*   PREF_MEM_LIMIT_...         -> offset 0x24                     */
/* ---------------------------------------------------------------- */
#define BAR0_REG_OFFSET                                     0x10U
#define BAR1_REG_OFFSET                                     0x14U
#define SEC_LAT_TIMER_OFFSET                                0x18U
#define SEC_STAT_IO_OFFSET                                  0x1CU
#define MEM_LIMIT_OFFSET                                    0x20U
#define PREF_MEM_LIMIT_OFFSET                               0x24U

/* BAR Enumeration Write Pattern */
#define BAR_ENUM_PATTERN                                    0xFFFFFFFFU

/* BAR Base Addresses */
#define BAR0_BASE_ADDR                                      0x00000000U
#define BAR1_BASE_ADDR                                      0x00000004U
#define BAR2_BASE_ADDR                                      0x20000000U
#define BAR3_BASE_ADDR                                      0x40000000U
#define BAR4_BASE_ADDR                                      0x60000000U
#define BAR5_BASE_ADDR                                      0x80000000U

/* ---------------------------------------------------------------- */
/* System-Level Configuration Registers                            */
/* ---------------------------------------------------------------- */
#define SYS_REG_0                                           0xE690000CU
#define SYS_REG_1                                           0xE6900010U
#define SYS_REG_2                                           0xE6900014U
#define SYS_REG_3                                           0xE6900018U
#define SYS_REG_4                                           0xE6900030U
#define SYS_REG_5                                           0xE6900034U

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
/* Extern Function Declarations                                     */
/* ---------------------------------------------------------------- */
extern void link_training_dm0_x4(int width);
extern void link_training_dm1_x4(int width);
extern void mem_base_program_dm0_x4(void);
extern void mem_base_program_dm1_x4(void);
extern void non_secure_prot_nic(void);
extern void wait_on(int count);
extern void finish(int status);
extern unsigned int set_data(unsigned int data, int start_bit, int end_bit, unsigned int value);
extern unsigned int read_pcie_slv0_reg(unsigned int offset);
extern void write_pcie_slv0_reg(unsigned int offset, unsigned int value);
extern unsigned int read_pcie_slv1_reg(unsigned int offset);
extern void write_pcie_slv1_reg(unsigned int offset, unsigned int value);

/* ---------------------------------------------------------------- */
/* Macros - No specific macros were supplied in the input.          */
/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
/* Arrays - No specific arrays were supplied in the input.          */
/* ---------------------------------------------------------------- */
