// Author - AI Force 2.3. 17-Sep-2025 04:28 IST
// (EMBENGG-SYSAPPS)

#ifndef USB_HOST_ENUMERATION_FS_H
#define USB_HOST_ENUMERATION_FS_H

#include "framework.h"

int usb_host_enumeration_fs_init(const TestsItem *cfg);
int usb_host_enumeration_fs_run(const TestsItem *cfg, TestOutput *out);
int usb_host_enumeration_fs_teardown(const TestsItem *cfg);

#endif /* USB_HOST_ENUMERATION_FS_H */
