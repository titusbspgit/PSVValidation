#!/usr/bin/env python3
"""Temporary generator script - will be deleted after use"""
import openpyxl
from openpyxl.styles import Font, PatternFill, Alignment
from datetime import datetime, timezone, timedelta
import json
import os

json_data = [{"index":1,"ss_module":"GPIO","test_case_name":"gpio_reg_wr_rd_test","feature":"Register Read/Write Validation","test_description":"This test validates the GPIO GP0 register block by performing two checks: (1) Reset value verification - reads each GPIO register and verifies the data matches the expected default reset values after masking read-only bits. (2) Write/Read verification - writes six different test patterns (all-ones, alternating bits, mixed patterns) to each writable GPIO register, reads back the values, and verifies correctness by accounting for read masks, write masks, and default values of non-writable bit fields. The test covers registers gp0_gpio_8, gp0_gpio_9, and gp0_gpio_10. The test reports PASS if all default value checks and write/read checks succeed, otherwise FAIL.","test_steps":"1. Initialize the test environment...","impacted_registers":"gp0_gpio_8; gp0_gpio_9; gp0_gpio_10","validation_acceptance_criteria":"1. All GPIO registers must return their expected default reset values...","speed":"NA","mode":"NA","remarks":"The test uses six distinct data patterns..."},{"index":2,"ss_module":"GPIO","test_case_name":"test_gpio_level_sel_intr_en","feature":"GPIO Level Select Interrupt Enable","test_description":"This test validates the GPIO level-select interrupt enable functionality...","test_steps":"1. Enable the GIC interrupt line...","impacted_registers":"intr_en1; gp0_gpio_8; raw_stcr1","validation_acceptance_criteria":"1. For each of the 32 GPIO pins...","speed":"NA","mode":"NA","remarks":"The test exercises both active-high and active-low..."}]
print("Script ready")
