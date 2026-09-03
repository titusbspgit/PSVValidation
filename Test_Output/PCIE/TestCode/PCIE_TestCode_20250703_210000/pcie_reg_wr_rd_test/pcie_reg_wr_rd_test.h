// Author - AI Force 2.3. 03-Jul-2025 21:00 IST
// (EMBENGG-SYSAPPS)

#ifndef PCIE_REG_WR_RD_TEST_H
#define PCIE_REG_WR_RD_TEST_H

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include <pcie.h>

int pcie_reg_wr_rd_test_init(const TestsItem *cfg);
int pcie_reg_wr_rd_test_run(const TestsItem *cfg, TestOutput *out);
int pcie_reg_wr_rd_test_teardown(const TestsItem *cfg);
void chk_rst_val(unsigned int addr, unsigned int exp_val, unsigned int *err);
void chk_rst_val_phy(unsigned int addr, unsigned int exp_val, unsigned int *err);
void chk_rd_wr(unsigned int addr, unsigned int wr_val, unsigned int write_mask, unsigned int *err);
void chk_rd_wr_phy(unsigned int addr, unsigned int wr_val, unsigned int write_mask, unsigned int *err);

#endif /* PCIE_REG_WR_RD_TEST_H */
