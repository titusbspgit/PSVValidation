/
 * usb_fs_device_bulk_transfer_test.h
 *
 * PSV testcase header: USB Full-Speed Device Bulk Transfer test
 /

#ifndef USB_FS_DEVICE_BULK_TRANSFER_TEST_H
#define USB_FS_DEVICE_BULK_TRANSFER_TEST_H

#include <test.h>

/ Framework entry-point prototypes */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg);
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out);
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem cfg);

/
 * IRQ handler - must have external linkage for vector table registration.
 * Declared here so the vector table or startup code can reference it.
 /
void Default_IRQHandler(void);

#endif / USB_FS_DEVICE_BULK_TRANSFER_TEST_H */
