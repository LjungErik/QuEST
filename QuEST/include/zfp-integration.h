#ifndef ZFP_INTEGRATION_H
#define ZFP_INTEGRATION_H

#include <stdbool.h>

#include "compression.h"
#include "QuEST_precision.h"
#include "zfp.h"

#define LOSSLESS_MODE 'R'
#define ACCURACY_MODE 'a'
#define PERCISION_MODE 'p'
#define RATE_MODE 'r'

typedef struct ZFPConfig {
    unsigned int dimensions;
    size_t nx;
    size_t ny;
    size_t nz;
    size_t nw;
    char mode;
    double tolerance;
    uint precision;
    double rate;
    zfp_exec_policy exec;
    zfp_type type;
} ZFPConfig;

CompressionImp zfpCreate(ZFPConfig config);
void zfpDestroy(CompressionImp imp);

size_t zfpMaxSize(void *config);

void zfpCompress(void *config, CompressedBlock* out_block, RawDataBlock* in_block);

void zfpDecompress(void *config, CompressedBlock* in_block, RawDataBlock* out_block);

#endif