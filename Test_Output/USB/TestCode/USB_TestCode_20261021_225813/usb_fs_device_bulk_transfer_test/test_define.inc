// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>
#include <stdio.h>
#include <stdint.h>
#include "usb.h"
#include "log.h"
#include "mmio.h"
#include "reg_access.h"
#include "gic_funcs.h"

// Runtime condition 2: nic_programming prototype
void nic_programming(void);

// Runtime condition 2: interrupt pending variable declared volatile
volatile int int_pend;

// DV global: event_counter
volatile unsigned int event_counter;
