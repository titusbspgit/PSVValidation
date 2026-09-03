// Author - AI Force 2.3. 03-Jul-2025 18:12 IST
// (EMBENGG-SYSAPPS)

/*
 * pcie_dma_write_test.h
 *
 * Header file for pcie_dma_write_test
 */

#ifndef PCIE_DMA_WRITE_TEST_H
#define PCIE_DMA_WRITE_TEST_H

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

int pcie_dma_write_test_init(const TestsItem *cfg);
int pcie_dma_write_test_run(const TestsItem *cfg, TestOutput *out);
int pcie_dma_write_test_teardown(const TestsItem *cfg);

#endif /* PCIE_DMA_WRITE_TEST_H */
