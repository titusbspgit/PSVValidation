// Author - AI Force 2.3. 30-Jun-2025 16:15 IST
// (EMBENGG-SYSAPPS)

#ifndef MIPI_DSI_BASIC_TEST_H
#define MIPI_DSI_BASIC_TEST_H

#include "framework.h"

int mipi_dsi_basic_test_init(const TestsItem *cfg);
int mipi_dsi_basic_test_run(const TestsItem *cfg, TestOutput *out);
int mipi_dsi_basic_test_teardown(const TestsItem *cfg);

#endif /* MIPI_DSI_BASIC_TEST_H */
