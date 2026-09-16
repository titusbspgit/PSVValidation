// Author - AI Force 2.3. 17-Sep-2025 04:28 IST
// (EMBENGG-SYSAPPS)

#ifndef USB_HOST_ENUMERATION_LS_H
#define USB_HOST_ENUMERATION_LS_H

#include "framework.h"

int usb_host_enumeration_ls_init(const TestsItem *cfg);
int usb_host_enumeration_ls_run(const TestsItem *cfg, TestOutput *out);
int usb_host_enumeration_ls_teardown(const TestsItem *cfg);

#endif /* USB_HOST_ENUMERATION_LS_H */
