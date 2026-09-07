// Author - AI Force 2.3. 07-Jul-2025 17:08 IST
// (EMBENGG-SYSAPPS)

#ifndef PCIE_MEM_WR_RD_TEST_H
#define PCIE_MEM_WR_RD_TEST_H

/* Function prototypes for pcie_mem_wr_rd_test */
int pcie_mem_wr_rd_test_init(const TestsItem *cfg);
int pcie_mem_wr_rd_test_run(const TestsItem *cfg, TestOutput *out);
int pcie_mem_wr_rd_test_teardown(const TestsItem *cfg);

#endif /* PCIE_MEM_WR_RD_TEST_H */
