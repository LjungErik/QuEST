#ifndef FPC_INTEGRATION_H
#define FPC_INTEGRATION_H

#include <stdbool.h>

#include "QuEST_precision.h"
#include "compression.h"
#include "fpc.h"

#define FPC_NULL_BUFFER_EXIT_CODE -68

typedef struct FPCConfig {
    size_t block_size;
    long predsizem1;
} FPCConfig;

bool fpcValidateConfig(FPCConfig config);

CompressionImp fpcCreate(FPCConfig config);

void fpcDestroy(CompressionImp imp);

size_t fpcMaxSize(void *config);

void fpcCompress(void *config, CompressedBlock* out_block, DecompressedBlock* in_block);

void fpcDecompress(void *config, CompressedBlock* in_block, DecompressedBlock* out_block);

#endif