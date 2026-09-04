// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#ifndef PCIE_DEVICE_ENUMERATE_TEST_H
#define PCIE_DEVICE_ENUMERATE_TEST_H

int pcie_device_enumerate_test_init(const TestsItem *cfg);
int pcie_device_enumerate_test_run(const TestsItem *cfg, TestOutput *out);
int pcie_device_enumerate_test_teardown(const TestsItem *cfg);

#endif /* PCIE_DEVICE_ENUMERATE_TEST_H */
