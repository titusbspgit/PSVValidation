// Author - AI Force 2.3. 18-Jul-2025 01:42 IST
// (EMBENGG-SYSAPPS)

#ifndef MIPI_DPHY_IDI_TEST_H
#define MIPI_DPHY_IDI_TEST_H

#include "framework.h"

int mipi_dphy_idi_test_init(const TestsItem *cfg);
int mipi_dphy_idi_test_run(const TestsItem *cfg, TestOutput *out);
int mipi_dphy_idi_test_teardown(const TestsItem *cfg);

#endif /* MIPI_DPHY_IDI_TEST_H */
