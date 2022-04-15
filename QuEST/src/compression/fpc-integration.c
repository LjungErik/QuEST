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

void fpcCompress(void *config, CompressedBlock* out_block, RawDataBlock* in_block) {
    
    FPCConfig *fpc_conf = (FPCConfig*)config;
    size_t fpcsize;
    void* buffer;
    size_t buffer_size;

    if (in_block->tmp_storage != NULL) {
         /* Use temporary block for dynamic memory allocation */
        buffer = in_block->tmp_storage;
        buffer_size = in_block->tmp_max_size;
    } else {
       /* Use the assigned memory in this case */
        buffer = out_block->data;
        buffer_size = out_block->max_size;
    }

    if (buffer == NULL) {
        printf("PANIC: No valid buffer exists!");
        exit(FPC_NULL_BUFFER_EXIT_CODE);
    }

    /* Perform compression here */
    FPC_MEM out_mem = {
        .ptr = buffer,
        .pos = 0,
        .length = buffer_size
    };

    FPC_MEM in_mem = {
        .ptr = in_block->data,
        .pos = 0,
        .length = in_block->size
    };

    
    fpcsize = fpc_compress(fpc_conf->predsizem1, &in_mem, &out_mem);  


    printf("[%p] - Compressed fpcsize: %li, buffer_size: %li\n", in_block->data, fpcsize, buffer_size);

    /* Handling Dynamic Allocation */
    if (buffer != out_block->data) {
        /* Check if block is allocated already or not */
        if (out_block->data == NULL) {
            /* No memory block exists */
            out_block->data = malloc(fpcsize);
        } else if(out_block->size != fpcsize) {
            /* Reallocate memory block */
            out_block->data = realloc(out_block->data, fpcsize);
        }
        /* Copy all the data from the buffer to the out block */
        memcpy(out_block->data, buffer, fpcsize);
    }

    out_block->size = fpcsize;
    out_block->n_values = in_block->n_values;
}

void fpcDecompress(void *config, CompressedBlock* in_block, RawDataBlock* out_block) {
    /* write the code to decompress the data blocks */

    printf("[%p] - Decompressing, input block size: %li\n", out_block->data, in_block->size);

    if (in_block->data != NULL) {
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
        
    } else {
        memset(out_block->data, 0, out_block->size);
    }

    out_block->size = in_block->n_values * sizeof(*(out_block->data));
    //printf("out_block->size: %li\n", out_block->size);
    out_block->n_values = in_block->n_values;
}