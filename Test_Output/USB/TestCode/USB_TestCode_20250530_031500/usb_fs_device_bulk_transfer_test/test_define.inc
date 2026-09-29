// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

/*
 * test_define.inc
 * Test Case: USB_FS_Device_Bulk_Transfer_test
 * Purpose: Supporting data, headers, macros, and arrays for USB FS Device
 *          Bulk Transfer FV testcase.
 */

#include "test.h"

#include <stdio.h>
#include <stdlib.h>
#include "usb.h"

#include "log.h"
#include "mmio.h"
#include "reg_access.h"

/* Arrays from Meta TestPlan JSON */
unsigned int buf_data[16];
