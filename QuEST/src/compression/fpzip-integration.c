#include <stdlib.h>
#include <stdio.h>

#include "fpzip-integration.h"

static void _fpzipConfigure(FPZIPConfig *fpzip_conf, FPZ* fpz) {
    fpz->type = fpzip_conf->type;
    fpz->prec = fpzip_conf->precision;
    fpz->nx = fpzip_conf->nx;
    fpz->ny = fpzip_conf->ny;
    fpz->nz = fpzip_conf->nz;
    fpz->nf = fpzip_conf->nw;
}

bool fpzipValidateConfig(FPZIPConfig config) {
    if (config.precision < 0) {
        fprintf(stderr, "Invalid precision\n");
        return false;
    }

    return true;
}

CompressionImp fpzipCreate(FPZIP config) {
    CompressionImp imp;
    FPZIPConfig *fpzip_conf = malloc(sizeof(FPZIPConfig));
    (*fpzip_conf)  = config;

    imp.config = fpzip_config;
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

    /* temporary solution TODO: fix this*/
    return fpz_conf.nx;
}

void fpzipCompress(void *config, CompressedBlock* out_block, RawDataBlock* in_block) {
    /* write the code to compress the data blocks */
}

void fpzipDecompress(void *config, CompressedBlock* in_block, RawDataBlock* out_block) {
    /* write the code to decompress the data blocks */
}