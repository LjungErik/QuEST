#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "fpzip.h"
#include "fpzip-integration.h"

#define MINIMUM_BUFFER_SIZE 576

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

void fpzipDestroy(CompressionImp imp) {
    free(imp.config);
}

size_t fpzipMaxSize(void *config) {
    FPZIPConfig *fpz_conf = (FPZIPConfig*)config;

    size_t count = (size_t)fpz_conf->nx * fpz_conf->ny * fpz_conf->nz * fpz_conf->nw;

    if (count < MINIMUM_BUFFER_SIZE) {
        count = MINIMUM_BUFFER_SIZE;
    }


    size_t size = (fpz_conf->type == FPZIP_TYPE_FLOAT ? sizeof(float) : sizeof(double));

    return count * size;
}

void fpzipCompress(void *config, CompressedBlock* out_block, DecompressedBlock* in_block) {
    FPZIPConfig *fpz_conf = (FPZIPConfig*)config;
    FPZ* fpz;
    size_t fpzsize;

    if (out_block->data == NULL) {
        printf("PANIC: No valid buffer exists!");
        exit(FPZIP_NULL_BUFFER_EXIT_CODE);
    }

    fpz = fpzip_write_to_buffer(out_block->data, out_block->max_size);
    _fpzipConfigure(fpz_conf, fpz);

    fpzsize = fpzip_write(fpz, in_block->data);
    if (!fpzsize) {
        fprintf(stderr, "compression failed: %s\n", fpzip_errstr[fpzip_errno]);
        exit(-1);
    }

    out_block->size = fpzsize;
    out_block->n_values = in_block->n_values;

    fpzip_write_close(fpz);
}

void fpzipDecompress(void *config, CompressedBlock* in_block, DecompressedBlock* out_block) {
    /* write the code to decompress the data blocks */
    FPZIPConfig *fpz_conf = (FPZIPConfig*)config;
    FPZ* fpz;

    fpz = fpzip_read_from_buffer(in_block->data);
    _fpzipConfigure(fpz_conf, fpz);

    if (in_block->data == NULL) {
        printf("PANIC: No valid input exists!");
        exit(FPZIP_NULL_BUFFER_EXIT_CODE);
    }

    if (!fpzip_read(fpz, out_block->data)) {
        fprintf(stderr, "decompression failed: %s\n", fpzip_errstr[fpzip_errno]);
        exit(-1);
    }
    fpzip_read_close(fpz);

    out_block->size = in_block->n_values * sizeof(*(out_block->data));
    out_block->n_values = in_block->n_values;
}