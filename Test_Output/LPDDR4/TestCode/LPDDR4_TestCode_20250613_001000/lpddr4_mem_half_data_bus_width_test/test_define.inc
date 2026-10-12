// Author - AI Force 2.3. Date in IST
// (EMBENGG-SYSAPPS)

#include <test.h>

// Meta Headers (preserved in first-seen order)
#include <stdio.h>
#include <stdlib.h>
#include "test_common.h"
#include "lpddr4.h"

// Framework headers for register access and logging
#include "log.h"
#include "mmio.h"
#include "reg_access.h"

// Meta Macros:
// APS_DRAM - Preprocessor conditional compilation flag for APS DRAM configuration;
//            externally defined via compiler command-line.
// MPS_DRAM - Preprocessor conditional compilation flag for MPS DRAM configuration;
//            externally defined via compiler command-line.
// SG2667  - Preprocessor conditional compilation flag for Speed Grade 2667 selection;
//            externally defined via compiler command-line.
// SG2133  - Preprocessor conditional compilation flag for Speed Grade 2133 selection;
//            externally defined via compiler command-line.
// A53_INITIATOR - Preprocessor conditional compilation flag for A53 initiator selection;
//                  externally defined via compiler command-line.
// AI_INITIATOR  - Preprocessor conditional compilation flag for AI initiator selection;
//                  externally defined via compiler command-line.
// DSP_INITIATOR - Preprocessor conditional compilation flag for DSP initiator selection;
//                  externally defined via compiler command-line.

// Meta Arrays: NA
