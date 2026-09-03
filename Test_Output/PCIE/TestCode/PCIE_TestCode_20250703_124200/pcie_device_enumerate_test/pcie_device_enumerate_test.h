/*
 * pcie_device_enumerate_test.h
 *
 * Header file for pcie_device_enumerate_test
 */

#ifndef PCIE_DEVICE_ENUMERATE_TEST_H
#define PCIE_DEVICE_ENUMERATE_TEST_H

#include <stdlib.h>
#include <stdio.h>
#include <test_common.h>
#include "pcie.h"

int pcie_device_enumerate_test_init(const TestsItem *cfg);
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput *out);
int pcie_device_enumerate_test_teardown(const TestsItem *cfg);

#endif /* PCIE_DEVICE_ENUMERATE_TEST_H */
