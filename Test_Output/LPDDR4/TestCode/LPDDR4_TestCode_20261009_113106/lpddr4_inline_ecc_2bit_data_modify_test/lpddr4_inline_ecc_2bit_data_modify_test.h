// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#ifndef LPDDR4_INLINE_ECC_2BIT_DATA_MODIFY_TEST_H
#define LPDDR4_INLINE_ECC_2BIT_DATA_MODIFY_TEST_H

#include <test.h>

int lpddr4_inline_ecc_2bit_data_modify_test_init(const TestsItem *cfg);

int lpddr4_inline_ecc_2bit_data_modify_test_run(
    const TestsItem *cfg,
    TestOutput *out
);

int lpddr4_inline_ecc_2bit_data_modify_test_teardown(
    const TestsItem *cfg
);

// Helper function prototypes from Meta
void memory_init_scrb(unsigned long int address);
void Default_IRQHandler(void);

#endif /* LPDDR4_INLINE_ECC_2BIT_DATA_MODIFY_TEST_H */
