/*
 * usb_fs_device_bulk_transfer_test.h
 *
 * PSV testcase header: USB FS Device Bulk Transfer Test
 * Auto-generated from DV-derived Meta TestPlan.
 /

#ifndef USB_FS_DEVICE_BULK_TRANSFER_TEST_H
#define USB_FS_DEVICE_BULK_TRANSFER_TEST_H

#include <test.h>

/ Framework entry-point prototypes */
int usb_fs_device_bulk_transfer_test_init(const TestsItem *cfg);
int usb_fs_device_bulk_transfer_test_run(const TestsItem *cfg, TestOutput *out);
int usb_fs_device_bulk_transfer_test_teardown(const TestsItem cfg);

/ IRQ handler -- externally visible for GIC vector table linkage /
void Default_IRQHandler(void);

#endif / USB_FS_DEVICE_BULK_TRANSFER_TEST_H */
