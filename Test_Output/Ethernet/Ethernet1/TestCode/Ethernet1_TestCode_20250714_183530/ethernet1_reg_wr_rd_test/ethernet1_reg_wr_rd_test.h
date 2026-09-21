// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#ifndef ETHERNET1_REG_WR_RD_TEST_H
#define ETHERNET1_REG_WR_RD_TEST_H

#include "framework.h"

int ethernet1_reg_wr_rd_test_init(const TestsItem *cfg);
int ethernet1_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out);
int ethernet1_reg_wr_rd_test_teardown(const TestsItem *cfg);

#endif /* ETHERNET1_REG_WR_RD_TEST_H */
