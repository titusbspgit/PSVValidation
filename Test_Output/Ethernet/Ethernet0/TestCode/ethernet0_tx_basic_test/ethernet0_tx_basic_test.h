// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#ifndef ETHERNET0_TX_BASIC_TEST_H
#define ETHERNET0_TX_BASIC_TEST_H

#include <test.h>

int ethernet0_tx_basic_test_init(const TestsItem *cfg);

int ethernet0_tx_basic_test_run(
 const TestsItem *cfg,
 TestOutput *out
);

int ethernet0_tx_basic_test_teardown(
 const TestsItem cfg
);

// Interrupt handler - must be visible to interrupt vector table
void Default_IRQHandler(void);

#endif / ETHERNET0_TX_BASIC_TEST_H */
