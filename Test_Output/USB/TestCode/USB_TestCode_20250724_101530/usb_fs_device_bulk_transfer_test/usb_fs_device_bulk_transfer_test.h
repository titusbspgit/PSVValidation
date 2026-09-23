// Author - AI Force 2.3. 24-Jul-2025 04:45 IST
// (EMBENGG-SYSAPPS)

#ifndef USB_FS_DEVICE_BULK_TRANSFER_TEST_H
#define USB_FS_DEVICE_BULK_TRANSFER_TEST_H

#include "framework.h"

int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg);
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out);
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem *cfg);

#endif /* USB_FS_DEVICE_BULK_TRANSFER_TEST_H */
