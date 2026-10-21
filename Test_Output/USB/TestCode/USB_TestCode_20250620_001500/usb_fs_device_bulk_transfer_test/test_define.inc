// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>
#include <stdio.h>
#include "usb.h"
#include "reg_access.h"
#include "gic_funcs.h"
#include "log.h"

// Runtime Condition 2: nic_programming prototype
void nic_programming(void);

// Runtime Condition 2: Declare interrupt pending variable as volatile
volatile int int_pend;

// Meta Types: event_counter at file scope
int event_counter;

// Meta Arrays: buf_data from test_case()
int buf_data[16];

// DWORD stride used in DV for clearing loops (jDWORD)
// MANUAL_REVIEW: DWORD macro must be defined in usb.h or project headers.
// If not defined there, this fallback applies:
#ifndef DWORD
#define DWORD 4
#endif
