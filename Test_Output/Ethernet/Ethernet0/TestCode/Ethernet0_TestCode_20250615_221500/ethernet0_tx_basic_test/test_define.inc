// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <ethernet0/ethernet0_def.h>
#include <test_common.h>
#include <ethernet0/ethernet0_offset.h>
#include <ethernet0/ethernet0_funcs.h>
#include <ethernet0/ethernet0_programming_sequence.h>
#include "log.h"
#include "reg_access.h"

// =========================================================================
// ATP COMPLIANCE NOTICE (Section 9.19 Ethernet)
// =========================================================================
// ATP Objective: Verify functional correctness of Ethernet controller by
// validating CSR register access, basic transmit path, multichannel
// multi-descriptor TX/RX path, Half-Duplex & Full-Duplex operation.
//
// ATP Hardware Topology: MIZAR SoC -> ETHERNET PHY -> External PHY Loopback
// -> Network Switch. Tools: RJ45 cable, network switch, UART USB cable,
// JTAG debugger, oscilloscope, Wireshark.
//
// ATP Functional Tests Required:
// - Ethernet TX and RX Data Path Test in Half-Duplex Mode
// - Ethernet TX and RX Data Path Test in Full-Duplex Mode
// - Ethernet Multichannel Multi-Descriptor TX/RX Test
//
// ATP Pass/Fail: Register accesses successful, reset/default values match
// specification, works in half-duplex & full-duplex mode, multichannel
// multi-descriptor TX/RX transfer completes successfully.
//
// MANUAL_REVIEW: [ATP-ETH-001] This testcase (ethernet0_tx_basic_test)
// covers only the basic TX transmit path on DMA CH0. It does NOT
// satisfy the full ATP Ethernet functional validation requirements.
// The following ATP gaps exist:
// 1. No external endpoint identification or hardware topology definition.
// 2. No RX frame validation (TX-only test).
// 3. No payload integrity verification.
// 4. No source/destination MAC address validation on received frames.
// 5. No external packet verification (e.g., Wireshark capture).
// 6. No duplex mode readback/status verification after configuration.
// 7. Pass criteria relies on interrupt completion counters only,
// not end-to-end data verification.
// Separate TX+RX data path tests with loopback and payload verification
// are required to satisfy ATP Section 9.19 fully.
// =========================================================================

// Meta Macros: These are conditional compilation macros.
// They are expected to be defined via build system / compiler flags:
// ETH_10M - Select 10 Mbps speed
// ETH_100M - Select 100 Mbps speed
// HALF_DUPLEX - Select Half-Duplex mode (default is Full-Duplex)
// SEL_ENET0 - Select Ethernet interface 0
// SEL_ENET1 - Select Ethernet interface 1
// SEL_ENET2 - Select Ethernet interface 2
// SEL_ENET3 - Select Ethernet interface 3
// DEBUG_DISPLAY - Enable verbose debug logging
