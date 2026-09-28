// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#ifndef LPDDR4_MEM_BASIC_WR_RD_TEST_H
#define LPDDR4_MEM_BASIC_WR_RD_TEST_H

#include "framework.h"

int lpddr4_mem_basic_wr_rd_test_init(const TestsItem *cfg);
int lpddr4_mem_basic_wr_rd_test_run(const TestsItem *cfg, TestOutput *out);
int lpddr4_mem_basic_wr_rd_test_teardown(const TestsItem cfg);

#endif /* LPDDR4_MEM_BASIC_WR_RD_TEST_H */
