// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include "test.h"
#include <stdio.h>
#include <stdlib.h>
#include "usb.h"
#include <stdint.h>
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

#define USB_HOST_FS_SLOT_CTX    0x08100000U
#define USB_HOST_FS_GFLADJ      0x0a07e000U

static unsigned int data_in[512];
static unsigned int data_out[512];
