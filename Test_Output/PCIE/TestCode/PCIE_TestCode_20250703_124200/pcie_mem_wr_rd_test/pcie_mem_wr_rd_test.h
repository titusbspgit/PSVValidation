// Author - AI Force 2.3. 03-Jul-2025 18:12 IST
// (EMBENGG-SYSAPPS)

/*
 * pcie_mem_wr_rd_test.h
 *
 * Header file for pcie_mem_wr_rd_test
 */

#ifndef PCIE_MEM_WR_RD_TEST_H
#define PCIE_MEM_WR_RD_TEST_H

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

int pcie_mem_wr_rd_test_init(const TestsItem *cfg);
int pcie_mem_wr_rd_test_run(const TestsItem *cfg, TestOutput *out);
int pcie_mem_wr_rd_test_teardown(const TestsItem *cfg);

#endif /* PCIE_MEM_WR_RD_TEST_H */
