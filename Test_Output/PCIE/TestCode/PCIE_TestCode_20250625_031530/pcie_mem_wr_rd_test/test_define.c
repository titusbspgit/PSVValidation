/*
 // Author - AI Force 1.3.2. Date 25-06-2025
 // (EMBENGG-SYSAPPS)
*/

/*
 * Test Case Name : pcie_mem_wr_rd_test
 * Purpose        : Definitions, macros, and register addresses for PCIe
 *                  memory write and read-back verification testcase.
 *                  Covers COHERENCY_CONTROL_3_OFF, TYPE1_DEV_ID_VEND_ID_REG,
 *                  TYPE1_STATUS_COMMAND_REG, and memory write-read helpers.
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
extern void bar_program_dm0_x4(void);
extern void bar_program_dm1_x4(void);
extern void bar_program_dm0_EP_x4(void);
extern void bar_program_dm1_EP_x4(void);
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

/* Memory write-read verification functions */
extern void pcie_slv0_mem_wr_rd(unsigned int addr, unsigned int data);
extern void pcie_slv1_mem_wr_rd(unsigned int addr, unsigned int data);

/* ---------------------------------------------------------------- */
/* Macros - No specific macros were supplied in the input.          */
/* ---------------------------------------------------------------- */

/* ---------------------------------------------------------------- */
/* Arrays - No specific arrays were supplied in the input.          */
/* ---------------------------------------------------------------- */
