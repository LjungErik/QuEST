#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fpzip.h"
#include "fpzip-integration.h"

static void _fpzipConfigure(FPZIPConfig *fpz_conf, FPZ* fpz) {
    fpz->type = fpz_conf->type;
    fpz->prec = fpz_conf->precision;
    fpz->nx = fpz_conf->nx;
    fpz->ny = fpz_conf->ny;
    fpz->nz = fpz_conf->nz;
    fpz->nf = fpz_conf->nw;
}

bool fpzipValidateConfig(FPZIPConfig config) {
    if (config.precision < 0) {
        fprintf(stderr, "Invalid precision\n");
        return false;
    }

    return true;
}

CompressionImp fpzipCreate(FPZIPConfig config) {
    CompressionImp imp;
    FPZIPConfig *fpzip_conf = malloc(sizeof(FPZIPConfig));
    (*fpzip_conf)  = config;

    imp.config = fpzip_conf;
    imp.max_size = fpzipMaxSize;
    imp.compress = fpzipCompress;
    imp.decompress = fpzipDecompress;

    return imp;
}

void fpzipDestory(CompressionImp imp) {
    free(imp.config);
}

size_t fpzipMaxSize(void *config) {
    FPZIPConfig *fpz_conf = (FPZIPConfig*)config;

    size_t count = (size_t)fpz_conf->nx * fpz_conf->ny * fpz_conf->nz * fpz_conf->nw;
    size_t size = (fpz_conf->type == FPZIP_TYPE_FLOAT ? sizeof(float) : sizeof(double));

    return count * size;
}

void fpzipCompress(void *config, CompressedBlock* out_block, RawDataBlock* in_block) {
    FPZIPConfig *fpz_conf = (FPZIPConfig*)config;
    FPZ* fpz;
    size_t fpzsize;
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
        exit(FPZIP_NULL_BUFFER_EXIT_CODE);
    }

    fpz = fpzip_write_to_buffer(buffer, buffer_size);
    _fpzipConfigure(fpz_conf, fpz);

    fpzsize = fpzip_write(fpz, in_block->data);
    if (!fpzsize) {
        fprintf(stderr, "compression failed: %s\n", fpzip_errstr[fpzip_errno]);
        exit(-1);
    }

    /* Handling Dynamic Allocation */
    if (buffer != out_block->data) {
        /* Check if block is allocated already or not */
        if (out_block->data == NULL) {
            /* No memory block exists */
            out_block->data = malloc(fpzsize);
        } else if(out_block->size != fpzsize) {
            /* Reallocate memory block */
            out_block->data = realloc(out_block->data, fpzsize);
        }
        /* Copy all the data from the buffer to the out block */
        memcpy(out_block->data, buffer, fpzsize);
    }

    out_block->size = fpzsize;

    fpzip_write_close(fpz);
}

void fpzipDecompress(void *config, CompressedBlock* in_block, RawDataBlock* out_block) {
    /* write the code to decompress the data blocks */
    FPZIPConfig *fpz_conf = (FPZIPConfig*)config;
    FPZ* fpz;

    if (in_block->data != NULL) {
        fpz = fpzip_read_from_buffer(in_block->data);
        _fpzipConfigure(fpz_conf, fpz);

        if (!fpzip_read(fpz, out_block->data)) {
            fprintf(stderr, "decompression failed: %s\n", fpzip_errstr[fpzip_errno]);
            exit(-1);
        }
        fpzip_read_close(fpz);
    } else {
        memset(out_block->data, 0, out_block->size);
    }

    out_block->n_values = in_block->n_values;
}