// Author - AI Force 2.3. 08-Jul-2025 09:18 IST
// (EMBENGG-SYSAPPS)

#ifndef PCIE_DMA_WRITE_TEST_H
#define PCIE_DMA_WRITE_TEST_H

#include "framework.h"

int pcie_dma_write_test_init(const TestsItem *cfg);
int pcie_dma_write_test_run(const TestsItem *cfg, TestOutput *out);
int pcie_dma_write_test_teardown(const TestsItem *cfg);

#endif /* PCIE_DMA_WRITE_TEST_H */
