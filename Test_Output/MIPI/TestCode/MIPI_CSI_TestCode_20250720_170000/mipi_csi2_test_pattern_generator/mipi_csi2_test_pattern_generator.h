// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#ifndef MIPI_CSI2_TEST_PATTERN_GENERATOR_H
#define MIPI_CSI2_TEST_PATTERN_GENERATOR_H

#include "framework.h"

int mipi_csi2_test_pattern_generator_init(const TestsItem *cfg);
int mipi_csi2_test_pattern_generator_run(const TestsItem *cfg, TestOutput *out);
int mipi_csi2_test_pattern_generator_teardown(const TestsItem *cfg);

#endif /* MIPI_CSI2_TEST_PATTERN_GENERATOR_H */
