// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#ifndef USB_FS_DEVICE_ISOCHRONOUS_TRANSFER_TEST_H
#define USB_FS_DEVICE_ISOCHRONOUS_TRANSFER_TEST_H

#include "framework.h"

int usb_fs_device_isochronous_transfer_test_init(const TestsItem *cfg);
int usb_fs_device_isochronous_transfer_test_run(const TestsItem *cfg, TestOutput *out);
int usb_fs_device_isochronous_transfer_test_teardown(const TestsItem *cfg);

#endif /* USB_FS_DEVICE_ISOCHRONOUS_TRANSFER_TEST_H */
