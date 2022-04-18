#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fpc.h"
#include "fpc-integration.h"

CompressionImp fpcCreate(FPCConfig config) {
    CompressionImp imp;
    FPCConfig *fpc_conf = malloc(sizeof(FPCConfig));
    (*fpc_conf)  = config;

    imp.config = fpc_conf;
    imp.max_size = fpcMaxSize;
    imp.compress = fpcCompress;
    imp.decompress = fpcDecompress;

    return imp;
}

bool fpcValidateConfig(FPCConfig config) {
    return config.block_size > 0 && config.predsizem1 > 0;
}

void fpcDestroy(CompressionImp imp) {
    free(imp.config);
}

size_t fpcMaxSize(void *config) {
    FPCConfig *fpc_conf = (FPCConfig*)config;

    size_t count = (size_t)fpc_conf->block_size;
    size_t size = sizeof(double);

    return count * size;
}

void fpcCompress(void *config, CompressedBlock* out_block, DecompressedBlock* in_block) {
    FPCConfig *fpc_conf = (FPCConfig*)config;
    size_t fpcsize;

    if (out_block->data == NULL) {
        printf("PANIC: No valid buffer exists!");
        exit(FPC_NULL_BUFFER_EXIT_CODE);
    }

    /* Perform compression here */
    FPC_MEM out_mem = {
        .ptr = out_block->data,
        .pos = 0,
        .length = out_block->max_size
    };

    FPC_MEM in_mem = {
        .ptr = in_block->data,
        .pos = 0,
        .length = in_block->size
    };

    fpcsize = fpc_compress(fpc_conf->predsizem1, &in_mem, &out_mem);

    out_block->size = fpcsize;
    out_block->n_values = in_block->n_values;
}

void fpcDecompress(void *config, CompressedBlock* in_block, DecompressedBlock* out_block) {
    /* write the code to decompress the data blocks */

    printf("[%p] - Decompressing, input block size: %li\n", out_block->data, in_block->size);

    if (in_block->data == NULL) {
        printf("PANIC: No valid input exists!");
        exit(FPC_NULL_BUFFER_EXIT_CODE);
    }

    FPC_MEM out_mem = {
        .ptr = out_block->data,
        .pos = 0,
        .length = out_block->size
    };

    FPC_MEM in_mem = {
        .ptr = in_block->data,
        .pos = 0,
        .length = in_block->size
    };

    printf("Decompressed size: %li, Out size: %li\n", in_block->size, out_block->size);

    fpc_decompress(&in_mem, &out_mem);

    out_block->size = in_block->n_values * sizeof(*(out_block->data));
    out_block->n_values = in_block->n_values;
}