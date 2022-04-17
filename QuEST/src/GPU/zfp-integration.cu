#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "zfp.h"
#include "zfp-integration.h"

void _zfpConfigure(ZFPConfig *zfp_conf, zfp_stream* zfp, zfp_field* field) {
    zfp_field_set_type(field, zfp_conf->type);
    switch (zfp_conf->dimensions) {
        case 1:
            zfp_field_set_size_1d(field, zfp_conf->nx);
            break;
        case 2:
            zfp_field_set_size_2d(field, zfp_conf->nx, zfp_conf->ny);
            break;
        case 3:
            zfp_field_set_size_3d(field, zfp_conf->nx, zfp_conf->ny, zfp_conf->nz);
            break;
        case 4:
            zfp_field_set_size_4d(field, zfp_conf->nx, zfp_conf->ny, zfp_conf->nz, zfp_conf->nw);
            break;
    }
    /* Add support for other modes */
    switch (zfp_conf->mode) {
        case LOSSLESS_MODE:
            zfp_stream_set_reversible(zfp);
            break;
        case ACCURACY_MODE:
            zfp_stream_set_accuracy(zfp, zfp_conf->tolerance);
            break;
        case PERCISION_MODE:
            zfp_stream_set_precision(zfp, zfp_conf->precision);
            break;
        case RATE_MODE:
            zfp_stream_set_rate(zfp, zfp_conf->rate, zfp_conf->type, zfp_conf->dimensions, zfp_false);
            break;
    }

    // Hard coded to only use cuda
    zfp_stream_set_execution(zfp, zfp_exec_cuda);
}

bool zfpValidateConfig(ZFPConfig config) {
    if (1 > config.dimensions || config.dimensions > 4 ) {
        fprintf(stderr, "Invalid dimensions\n");
        return false;
    }

    if (config.mode != LOSSLESS_MODE && config.mode != ACCURACY_MODE && 
        config.mode != PERCISION_MODE && config.mode != RATE_MODE) {
        fprintf(stderr, "Invalid mode\n");
        return false;
    }

    return true;
}

CompressionImp zfpCreate(ZFPConfig config) {
    CompressionImp imp = {};
    // ZFPConfig *zfp_conf = (ZFPConfig*) malloc(sizeof(ZFPConfig));
    // (*zfp_conf) = config;

    // imp.config = zfp_conf;
    // imp.max_size = zfpMaxSize;
    // imp.compress = zfpCompress;
    // imp.decompress = zfpDecompress;

    return imp;
}

void zfpDestroy(CompressionImp imp) {
    free(imp.config);
}

size_t zfpMaxSize(void *config) {
    ZFPConfig *zfp_conf = (ZFPConfig*)config;

    zfp_stream* zfp = zfp_stream_open(NULL);

    zfp_field* field = zfp_field_alloc();
    size_t max_n = 0;

    _zfpConfigure(zfp_conf, zfp, field);

    max_n = zfp_stream_maximum_size(zfp, field);

    zfp_field_free(field);
    zfp_stream_close(zfp);

    return max_n;
}

void zfpCompress(void *config, CompressedBlock* out_block, RawDataBlock* in_block) {
    ZFPConfig *zfp_conf = (ZFPConfig*)config;

    bitstream* stream;
    zfp_stream* zfp;
    zfp_field* field;
    size_t zfpsize;
    void* buffer;
    size_t buffer_size;

    zfp = zfp_stream_open(NULL); 
    field = zfp_field_alloc();

    zfp_field_set_pointer(field, in_block->data);

    _zfpConfigure(zfp_conf, zfp, field);

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
        exit(ZPF_NULL_BUFFER_EXIT_CODE);
    }

    stream = stream_open(buffer, buffer_size);

    zfp_stream_set_bit_stream(zfp, stream);

    zfpsize = zfp_compress(zfp, field);

    /* Handling Dynamic Allocation */
    if (buffer != out_block->data) {
        /* Check if block is allocated already or not */
        if (out_block->data == NULL) {
            /* No memory block exists */
            cudaMalloc(&(out_block->data), zfpsize);
        } else if(out_block->size != zfpsize) {
            /* Reallocate memory block */
            cudaFree(out_block->data);
            cudaMalloc(&(out_block->data), zfpsize);
        }
        /* Copy all the data from the buffer to the out block */
        cudaMemcpy(out_block->data, buffer, zfpsize, cudaMemcpyDeviceToDevice);
    }

    out_block->size = zfpsize;
    out_block->n_values = in_block->n_values;

    zfp_field_free(field);  
    zfp_stream_close(zfp);
    stream_close(stream);
}

void zfpDecompress(void *config, CompressedBlock* in_block, RawDataBlock* out_block) {
    ZFPConfig *zfp_conf = (ZFPConfig*)config;

    bitstream* stream;
    zfp_stream* zfp;
    zfp_field* field;

    /* Check if in block actually has data */
    if (in_block->data != NULL) {
        zfp = zfp_stream_open(NULL); 
        field = zfp_field_alloc();

        stream = stream_open(in_block->data, in_block->size);
        zfp_stream_set_bit_stream(zfp, stream);

        _zfpConfigure(zfp_conf, zfp, field);

        zfp_stream_rewind(zfp);

        zfp_field_set_pointer(field, out_block->data);

        zfp_decompress(zfp, field);

        zfp_field_free(field);  
        zfp_stream_close(zfp);
        stream_close(stream);
    } else {
        /* No data exists, clear RawDataBlock */
        cudaMemset(out_block->data, 0, out_block->size);
    }
    
    out_block->size = in_block->n_values * sizeof(*(out_block->data));
    out_block->n_values = in_block->n_values;
}