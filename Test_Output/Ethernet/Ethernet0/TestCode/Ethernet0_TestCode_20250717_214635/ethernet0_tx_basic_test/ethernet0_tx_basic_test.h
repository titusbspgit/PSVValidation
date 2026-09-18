// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#ifndef ETHERNET0_TX_BASIC_TEST_H
#define ETHERNET0_TX_BASIC_TEST_H

#include "framework.h"

int ethernet0_tx_basic_test_init(const TestsItem *cfg);
int ethernet0_tx_basic_test_run(const TestsItem *cfg, TestOutput *out);
int ethernet0_tx_basic_test_teardown(const TestsItem *cfg);

#endif /* ETHERNET0_TX_BASIC_TEST_H */
