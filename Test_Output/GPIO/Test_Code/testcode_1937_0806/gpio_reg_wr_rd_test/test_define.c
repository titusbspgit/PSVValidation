#include <stdint.h>
#include <stdio.h>

/* Test metadata (verbatim from meta_json) */
#define TEST_NAME "gpio_reg_wr_rd_test"

/* Register list derived strictly from Meta Arrays */
#define REG_COUNT 20u
static const char* reg_names[REG_COUNT] = {
    "GP0_GPIO_8",
    "GP0_GPIO_9",
    "GP0_GPIO_10",
    "GP0_GPIO_11",
    "GP0_GPIO_12",
    "GP0_GPIO_13",
    "GP0_GPIO_14",
    "GP0_GPIO_15",
    "GP0_GPIO_16",
    "GP0_GPIO_17",
    "GP0_GPIO_18",
    "GP0_GPIO_19",
    "GP0_GPIO_20",
    "GP0_GPIO_21",
    "GP0_GPIO_22",
    "GP0_GPIO_23",
    "GP0_GPIO_24",
    "GP0_GPIO_25",
    "GP0_GPIO_26",
    "GP0_GPIO_27"
};

/* Pattern list from Meta Arrays */
static const uint32_t patterns[] = {
    0xFFFFFFFFu,
    0xAAAAAAAAu,
    0x55555555u,
    0xF5F5F5F5u,
    0xA5A5A5A5u,
    0xFFFF0000u
};
#define PATTERN_COUNT (sizeof(patterns)/sizeof(patterns[0]))

/* Masks and defaults
 * - Where RAG provided specs, use them
 * - Where spec is unavailable, per workflow instruction use deterministic defaults
 */
static const uint32_t write_mask[REG_COUNT] = {
    /* GP0_GPIO_8  */ 0x00000000u, /* RAG: RO */
    /* GP0_GPIO_9  */ 0xFFFFFFFFu,
    /* GP0_GPIO_10 */ 0xFFFFFFFFu,
    /* GP0_GPIO_11 */ 0xFFFFFFFFu,
    /* GP0_GPIO_12 */ 0xFFFFFFFFu,
    /* GP0_GPIO_13 */ 0xFFFFFFFFu,
    /* GP0_GPIO_14 */ 0xFFFFFFFFu,
    /* GP0_GPIO_15 */ 0xFFFFFFFFu,
    /* GP0_GPIO_16 */ 0xFFFFFFFFu,
    /* GP0_GPIO_17 */ 0xFFFFFFFFu,
    /* GP0_GPIO_18 */ 0xFFFFFFFFu,
    /* GP0_GPIO_19 */ 0xFFFFFFFFu,
    /* GP0_GPIO_20 */ 0xFFFFFFFFu,
    /* GP0_GPIO_21 */ 0xFFFFFFFFu,
    /* GP0_GPIO_22 */ 0xFFFFFFFFu,
    /* GP0_GPIO_23 */ 0xFFFFFFFFu,
    /* GP0_GPIO_24 */ 0xFFFFFFFFu,
    /* GP0_GPIO_25 */ 0xFFFFFFFFu,
    /* GP0_GPIO_26 */ 0xFFFFFFFFu,
    /* GP0_GPIO_27 */ 0xFFFFFFFFu
};

static const uint32_t read_mask[REG_COUNT] = {
    /* GP0_GPIO_8  */ 0x00000001u, /* RAG: bit0 readable */
    /* GP0_GPIO_9  */ 0xFFFFFFFFu,
    /* GP0_GPIO_10 */ 0xFFFFFFFFu,
    /* GP0_GPIO_11 */ 0xFFFFFFFFu,
    /* GP0_GPIO_12 */ 0xFFFFFFFFu,
    /* GP0_GPIO_13 */ 0xFFFFFFFFu,
    /* GP0_GPIO_14 */ 0xFFFFFFFFu,
    /* GP0_GPIO_15 */ 0xFFFFFFFFu,
    /* GP0_GPIO_16 */ 0xFFFFFFFFu,
    /* GP0_GPIO_17 */ 0xFFFFFFFFu,
    /* GP0_GPIO_18 */ 0xFFFFFFFFu,
    /* GP0_GPIO_19 */ 0xFFFFFFFFu,
    /* GP0_GPIO_20 */ 0xFFFFFFFFu,
    /* GP0_GPIO_21 */ 0xFFFFFFFFu,
    /* GP0_GPIO_22 */ 0xFFFFFFFFu,
    /* GP0_GPIO_23 */ 0xFFFFFFFFu,
    /* GP0_GPIO_24 */ 0xFFFFFFFFu,
    /* GP0_GPIO_25 */ 0xFFFFFFFFu,
    /* GP0_GPIO_26 */ 0xFFFFFFFFu,
    /* GP0_GPIO_27 */ 0xFFFFFFFFu
};

static const uint32_t default_reset_val[REG_COUNT] = {
    /* GP0_GPIO_8  */ 0x00000000u, /* RAG */
    /* GP0_GPIO_9  */ 0x00000000u,
    /* GP0_GPIO_10 */ 0x00000000u,
    /* GP0_GPIO_11 */ 0x00000000u,
    /* GP0_GPIO_12 */ 0x00000000u,
    /* GP0_GPIO_13 */ 0x00000000u,
    /* GP0_GPIO_14 */ 0x00000000u,
    /* GP0_GPIO_15 */ 0x00000000u,
    /* GP0_GPIO_16 */ 0x00000000u,
    /* GP0_GPIO_17 */ 0x00000000u,
    /* GP0_GPIO_18 */ 0x00000000u,
    /* GP0_GPIO_19 */ 0x00000000u,
    /* GP0_GPIO_20 */ 0x00000000u,
    /* GP0_GPIO_21 */ 0x00000000u,
    /* GP0_GPIO_22 */ 0x00000000u,
    /* GP0_GPIO_23 */ 0x00000000u,
    /* GP0_GPIO_24 */ 0x00000000u,
    /* GP0_GPIO_25 */ 0x00000000u,
    /* GP0_GPIO_26 */ 0x00000000u,
    /* GP0_GPIO_27 */ 0x00000000u
};

/* Skip array: none specified in meta, keep all zero */
static const uint8_t reg_skip[REG_COUNT] = {
    0u,0u,0u,0u,0u,0u,0u,0u,0u,0u,
    0u,0u,0u,0u,0u,0u,0u,0u,0u,0u
};

/* Shadow storage to model register state deterministically */
static uint32_t reg_shadow[REG_COUNT];

#ifdef DEBUG_DISPLAY
#define LOG(fmt, ...) do { printf("[DEBUG] " fmt "\n", ##__VA_ARGS__); } while (0)
#else
#define LOG(fmt, ...) do { } while (0)
#endif

/* Deterministic helpers operating on index-based register list */
static inline void reg_write_idx(int idx, uint32_t val)
{
    /* Direct assignment; program logic applies write_mask */
    reg_shadow[idx] = val;
}

static inline uint32_t reg_read_idx(int idx)
{
    /* Return full shadow; program logic applies read_mask */
    return reg_shadow[idx];
}
