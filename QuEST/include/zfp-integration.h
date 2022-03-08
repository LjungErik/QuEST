#ifndef ZFP_INTEGRATION_H
#define ZFP_INTEGRATION_H

#include <stdbool.h>

#include "QuEST_precision.h"
#include "zfp.h"

typedef struct ZFPConfig {
    size_t n_blocks;
    size_t values_per_block;
    unsigned int dimensions;
    char mode;
    double rate;
    zfp_exec_policy exec;
} ZFPConfig;

#endif