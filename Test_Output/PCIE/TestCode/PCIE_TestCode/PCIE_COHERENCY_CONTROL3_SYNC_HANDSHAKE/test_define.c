#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

/* RAG mapping: DBI-relative offset for COHERENCY_CONTROL_3 within DSP PF0 PORT_LOGIC */
#ifndef PCIE_DSP_PF0_PORT_LOGIC_COHERENCY_CONTROL_3_OFF
#define PCIE_DSP_PF0_PORT_LOGIC_COHERENCY_CONTROL_3_OFF 0x8e8u
#endif

/* If impacted register macros are not provided by pcie.h, derive from DBI base + offset.
 * Absolute = <PCIE*_DBI_BASE> + 0x8e8. Provide DBI base via platform headers.
 */
#ifndef mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF
# ifdef PCIE0_DBI_BASE
#  define mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF (PCIE0_DBI_BASE + PCIE_DSP_PF0_PORT_LOGIC_COHERENCY_CONTROL_3_OFF)
# else
#  ifdef __GNUC__
#   warning "PCIE0_DBI_BASE is not defined; please define mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF in pcie.h or provide PCIE0_DBI_BASE"
#  endif
# endif
#endif

#ifndef mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF
# ifdef PCIE1_DBI_BASE
#  define mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF (PCIE1_DBI_BASE + PCIE_DSP_PF0_PORT_LOGIC_COHERENCY_CONTROL_3_OFF)
# else
#  ifdef __GNUC__
#   warning "PCIE1_DBI_BASE is not defined; please define mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF in pcie.h or provide PCIE1_DBI_BASE"
#  endif
# endif
#endif

/* Build-time role macro check for clarity */
#if !defined(DM0_RC) && !defined(DM1_RC) && !defined(DM0_EP) && !defined(DM1_EP)
# ifdef __GNUC__
#  warning "No role macro (DM0_RC/DM1_RC/DM0_EP/DM1_EP) defined; link training and config sequences may be skipped."
# endif
#endif

/* Impacted Registers Array (from Meta Impacted Registers) */
static const unsigned int impacted_registers[] = {
    mizar_PCIE0_DBI_DSP_COHERENCY_CONTROL_3_OFF,
    mizar_PCIE1_DBI_DSP_COHERENCY_CONTROL_3_OFF
};
static const unsigned int impacted_registers_count = 2u;

/* Skip Registers Array (from Meta Arrays: none provided) */
static const unsigned int skip_registers[] = { };
static const unsigned int skip_registers_count = 0u;
