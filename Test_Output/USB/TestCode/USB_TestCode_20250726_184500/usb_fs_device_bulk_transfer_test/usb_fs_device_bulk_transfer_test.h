/
 * File: usb_fs_device_bulk_transfer_test.h
 * Description: PSV testcase header - USB FS Device Bulk Transfer Test
 *****************************************************************************/

#ifndef USB_FS_DEVICE_BULK_TRANSFER_TEST_H
#define USB_FS_DEVICE_BULK_TRANSFER_TEST_H

#include <test.h>

int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg);
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out);
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem cfg);

// ISR handler - externally visible for vector table registration
void Default_IRQHandler(void);

#endif /* USB_FS_DEVICE_BULK_TRANSFER_TEST_H */
