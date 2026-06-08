/* Auto-generated test definitions for test_gpio_pedge_all_pads_en
 * Source: meta_json (verbatim). Do not modify.
 * Notes:
 * - Meta Headers: NA (no additional headers included here)
 * - Meta Macros: NA (no additional macros defined here)
 * - Arrays are constructed strictly from Meta Impacted Registers.
 * - RAG mapping unavailable for these macros; keep identifiers as-is.
 */

/* Impacted register identifiers (macros expected to be provided by platform headers/toolchain). */
const unsigned long int REG_POS_EDGE_EN[1]      = { POS_EDGE_EN };
const unsigned long int REG_INT_EN[1]           = { INT_EN };
const unsigned long int REG_INT_RAW_STAT[1]     = { INT_RAW_STAT };
const unsigned long int REG_INT_STAT_CLR[1]     = { INT_STAT_CLR };
const unsigned long int REG_GROUP_INT_ENABLE[1] = { GROUP_INT_ENABLE };
const unsigned long int REG_GROUP_INT_STATUS[1] = { GROUP_INT_STATUS };
const unsigned long int REG_DATA_OUT[1]         = { DATA_OUT };
const unsigned long int REG_DIR[1]              = { DIR };

/* Flat list of all impacted registers (ordered, deterministic). */
const unsigned long int REG_ALL[8] = {
    POS_EDGE_EN,
    INT_EN,
    INT_RAW_STAT,
    INT_STAT_CLR,
    GROUP_INT_ENABLE,
    GROUP_INT_STATUS,
    DATA_OUT,
    DIR
};

/* Skip Registers array
 * - No skip directives provided in meta_json; keep as empty placeholder.
 * - Ensure any future skipped registers appear ONLY here.
 */
const unsigned int SKIP_COUNT = 0u;
const unsigned long int SKIP_REGS[1] = { 0u };
