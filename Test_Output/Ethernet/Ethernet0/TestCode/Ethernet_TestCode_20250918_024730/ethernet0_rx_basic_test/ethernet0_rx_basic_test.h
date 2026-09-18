// Author - AI Force 2.3. 18-Sep-2025 08:17 IST
// (EMBENGG-SYSAPPS)

#ifndef ETHERNET0_RX_BASIC_TEST_H
#define ETHERNET0_RX_BASIC_TEST_H

#include "framework.h"

int ethernet0_rx_basic_test_init(const TestsItem *cfg);
int ethernet0_rx_basic_test_run(const TestsItem *cfg, TestOutput *out);
int ethernet0_rx_basic_test_teardown(const TestsItem *cfg);

#endif /* ETHERNET0_RX_BASIC_TEST_H */
