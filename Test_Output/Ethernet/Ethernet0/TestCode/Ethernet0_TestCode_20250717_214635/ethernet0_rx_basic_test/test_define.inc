// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * Test Case: ethernet0_rx_basic_test
 * Purpose:   Basic Ethernet RX path validation. Configures GIC, speed/interface,
 *            MAC/MTL/DMA registers, preloads RX descriptors, starts RX on CH0,
 *            loops waiting for transfer-complete interrupts, and validates via
 *            IRQ handler DMA status clearing.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ethernet0/ethernet0_def.h>
#include <test_common.h>
#include <ethernet0/ethernet0_offset.h>
#include <ethernet0/ethernet0_funcs.h>
#include <ethernet0/ethernet0_programming_sequence.h>

/*
 * Macros: Conditional compilation flags for speed, interface, and duplex mode.
 * These macros are expected to be defined externally via build configuration.
 *
 * ETH_10M       - Select 10 Mbps speed
 * ETH_100M      - Select 100 Mbps speed
 * SEL_ENET0     - Select Ethernet interface 0
 * SEL_ENET1     - Select Ethernet interface 1
 * SEL_ENET2     - Select Ethernet interface 2
 * SEL_ENET3     - Select Ethernet interface 3
 * HALF_DUPLEX   - Select half-duplex mode
 * DEBUG_DISPLAY - Enable debug display output
 */

/*
 * Timeout constant for RX interrupt polling loop.
 * MANUAL_REVIEW: Adjust this value based on the target platform timing.
 */
#ifndef ETHERNET0_RX_TIMEOUT
#define ETHERNET0_RX_TIMEOUT 100000U
#endif
