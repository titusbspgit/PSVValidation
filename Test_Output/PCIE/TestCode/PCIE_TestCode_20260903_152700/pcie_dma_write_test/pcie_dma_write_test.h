// Author - AI Force 2.3. 03-Sep-2026 15:27 IST
// (EMBENGG-SYSAPPS)

#ifndef PCIE_DMA_WRITE_TEST_H
#define PCIE_DMA_WRITE_TEST_H

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

int pcie_dma_write_test_init(const TestsItem *cfg);
int pcie_dma_write_test_run(const TestsItem *cfg, TestOutput *out);
int pcie_dma_write_test_teardown(const TestsItem *cfg);
void Default_IRQHandler(void);

#endif /* PCIE_DMA_WRITE_TEST_H */
