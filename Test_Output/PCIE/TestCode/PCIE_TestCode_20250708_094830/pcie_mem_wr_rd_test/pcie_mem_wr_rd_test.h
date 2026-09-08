// Author - AI Force 2.3. 08-Jul-2025 09:18 IST
// (EMBENGG-SYSAPPS)

#ifndef PCIE_MEM_WR_RD_TEST_H
#define PCIE_MEM_WR_RD_TEST_H

#include "framework.h"

int pcie_mem_wr_rd_test_init(const TestsItem *cfg);
int pcie_mem_wr_rd_test_run(const TestsItem *cfg, TestOutput *out);
int pcie_mem_wr_rd_test_teardown(const TestsItem *cfg);

#endif /* PCIE_MEM_WR_RD_TEST_H */
