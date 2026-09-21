// Author - AI Force 2.3. 21-Sep-2025 20:45 IST
// (EMBENGG-SYSAPPS)

#ifndef ETHERNET2_REG_WR_RD_TEST_H
#define ETHERNET2_REG_WR_RD_TEST_H

#include "framework.h"

int ethernet2_reg_wr_rd_test_init(const TestsItem *cfg);
int ethernet2_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out);
int ethernet2_reg_wr_rd_test_teardown(const TestsItem *cfg);

#endif /* ETHERNET2_REG_WR_RD_TEST_H */
