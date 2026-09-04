// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#ifndef PCIE_REG_WR_RD_TEST_H
#define PCIE_REG_WR_RD_TEST_H

int pcie_reg_wr_rd_test_init(const TestsItem *cfg);
int pcie_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out);
int pcie_reg_wr_rd_test_teardown(const TestsItem *cfg);

#endif /* PCIE_REG_WR_RD_TEST_H */
