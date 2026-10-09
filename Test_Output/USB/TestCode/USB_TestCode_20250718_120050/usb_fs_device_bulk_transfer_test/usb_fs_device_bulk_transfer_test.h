// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#ifndef USB_FS_DEVICE_BULK_TRANSFER_TEST_H
#define USB_FS_DEVICE_BULK_TRANSFER_TEST_H

#include <test.h>

int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg);

int usb_fs_device_bulk_transfer_test_run(
    const TestsItem *cfg,
    TestOutput *out
);

int usb_fs_device_bulk_transfer_test_teardown(
    const TestsItem *cfg
);

// IRQ handler prototype
void Default_IRQHandler(void);

// set_configuration helper prototype
void set_configuration(int trb_address, int parameter0, int parameter1, int cmd);

#endif /* USB_FS_DEVICE_BULK_TRANSFER_TEST_H */
