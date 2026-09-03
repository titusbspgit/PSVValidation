/*
 * pcie_device_enumerate_test.h
 *
 * Header file for pcie_device_enumerate_test
 */

#ifndef PCIE_DEVICE_ENUMERATE_TEST_H
#define PCIE_DEVICE_ENUMERATE_TEST_H

#include <stdio.h>
#include <stdint.h>
#include "framework.h"
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

int pcie_device_enumerate_test_init(const TestsItem *cfg);
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput *out);
int pcie_device_enumerate_test_teardown(const TestsItem *cfg);

#endif /* PCIE_DEVICE_ENUMERATE_TEST_H */
