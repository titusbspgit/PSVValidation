// Author - AI Force 2.3. 18-Jul-2025 01:42 IST
// (EMBENGG-SYSAPPS)

#ifndef MIPI_CSI2_RB_REG_WR_RD_TEST_H
#define MIPI_CSI2_RB_REG_WR_RD_TEST_H

#include "framework.h"

int mipi_csi2_rb_reg_wr_rd_test_init(const TestsItem *cfg);
int mipi_csi2_rb_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out);
int mipi_csi2_rb_reg_wr_rd_test_teardown(const TestsItem *cfg);

#endif /* MIPI_CSI2_RB_REG_WR_RD_TEST_H */
