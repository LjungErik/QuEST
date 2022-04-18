#ifndef ZFP_INTEGRATION_H
#define ZFP_INTEGRATION_H

#include <stdbool.h>

#include "QuEST_precision.h"
#include "compression.h"
#include "zfp.h"
#include "zfp-config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define LOSSLESS_MODE 'R'
#define ACCURACY_MODE 'a'
#define PERCISION_MODE 'p'
#define RATE_MODE 'r'

#define ZPF_NULL_BUFFER_EXIT_CODE -66

bool zfpValidateConfig(ZFPConfig config);

CompressionImp zfpCreate(ZFPConfig config);

void zfpDestroy(CompressionImp imp);

size_t zfpMaxSize(void *config);

void zfpCompress(void *config, CompressedBlock* out_block, RawDataBlock* in_block);

void zfpDecompress(void *config, CompressedBlock* in_block, RawDataBlock* out_block);

#ifdef __cplusplus
}
#endif

#endif