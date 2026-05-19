#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* Macros derived from Meta JSON (unchanged semantics) */
#define SYNC_REG_ADDR   0xE6004100U
#define SII_STATUS_OFF  0x000000C0U
#define READY_MASK      0x000000D1U
#define READY_VAL       0x000000D1U
#define HANDSHAKE_START 0x11111111U
#define HANDSHAKE_DONE  0x12345678U

/* Impacted Registers (from Meta Impacted Registers) */
const unsigned int pcie_coh_ctrl_regs[2] = {
    mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF,
    mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF
};

/* Bitfield masks for COHERENCY_CONTROL_3_OFF as per Meta Steps and RAG */
#define AW_CACHE_MASK   (0xFU << 11)  /* [14:11] */
#define AR_CACHE_MASK   (0xFU << 3)   /* [6:3]  */
#define AW2_CACHE_MASK  (0xFU << 27)  /* [30:27] */
#define AR2_CACHE_MASK  (0xFU << 19)  /* [22:19] */
