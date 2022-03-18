#ifndef FPZIP_INTEGRATION_H
#define FPZIP_INTEGRATION_H

#include <stdbool.h>

#include "QuEST_precision.h"
#include "compression.h"
#include "fpzip.h"

typedef struct FPZIPConfig {
    int nx;
    int ny;
    int nz;
    int nw;
    int precision;
    int type;
} FPZIPConfig;

bool fpzipValidateConfig(FPZIPConfig config);

CompressionImp fpzipCreate(FPZIPConfig config);

void fpzipDestroy(CompressionImp imp);

size_t fpzipMaxSize(void *config);

void fpzipCompress(void *config, CompressedBlock* out_block, RawDataBlock* in_block);

void fpzipDecompress(void *config, CompressedBlock* in_block, RawDataBlock* out_block);

#endif