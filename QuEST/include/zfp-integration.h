#ifndef ZFP_INTEGRATION_H
#define ZFP_INTEGRATION_H

#include <stdbool.h>

#include "compression.h"
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

size_t zfpMaxSize(void *config, RawDataBlock *block);

void zfpCompress(void *config, CompressedBlock* out_block, RawDataBlock* in_block);

void zfpDecompress(void *config, CompressedBlock* in_block, RawDataBlock* out_block);

#endif