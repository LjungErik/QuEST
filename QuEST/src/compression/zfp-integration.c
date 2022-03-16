#include "zfp-integration.h"

size_t zfpMaxSize(void *config, RawDataBlock *block) {
    ZFPConfig zfp_conf = (ZFPConfig*)config;

    zfp_stream* zfp = zfp_stream_open(NULL);
    zfp_field* field = zfp_field_alloc();
    size_t max_n = 0;

    zfp_field_set_type(field, zfp_type_qreal);
    zfp_field_set_size_1d(field, block->n_values);

    /* Add support for other moddes */
    switch (zfp_conf->mode) {
        case 'r':
            zfp_stream_set_rate(zfp, zfp_conf->rate, zfp_type_qreal, zfp_conf->dimensions, zfp_false);
    }

    zfp_stream_set_execution(zfp, mem->exec);

    max_n = zfp_stream_maximum_size(zfp, field);

    zfp_field_free(field);
    zfp_stream_close(zfp);

    return max_n

}

void zfpCompress(void *config, CompressedBlock* out_block, RawDataBlock* in_block) {
    ZFPConfig zfp_conf = (ZFPConfig*)config;

    bitstream* stream;
    zfp_stream* zfp;
    zfp_field* field;
    size_t zfpsize;

    zfp = zfp_stream_open(NULL); 
    field = zfp_field_alloc();

    zfp_field_set_pointer(field, in_block->data);
    zfp_field_set_type(field, zfp_type_qreal);
    // Add support for handling multi dimensional
    zfp_field_set_size_1d(field, in_block->n_values);

    /* Add support for other moddes */
    switch (zfp_conf->mode) {
        case 'r':
            zfp_stream_set_rate(zfp, zfp_conf->rate, zfp_type_qreal, zfp_conf->dimensions, zfp_false);
    }

    zfp_stream_set_execution(zfp, zfp_conf->exec);

    stream = stream_open(out_block->data, out_block->max_size);

    zfp_stream_set_bit_stream(zfp, stream);

    size_t zfpsize = zfp_compress(zfp, field);
    out_block->size = zfpsize;
    in_block->n_values = 0;

    zfp_field_free(field);  
    zfp_stream_close(zfp);
    stream_close(stream);
}

void zfpDecompress(void *config, CompressedBlock* in_block, RawDataBlock* out_block) {

    bitstream* stream;
    zfp_stream* zfp;
    zfp_field* field;

    zfp = zfp_stream_open(NULL); 
    field = zfp_field_alloc();

    stream = stream_open(in_blocks->data, in_blocks->size);
    zfp_stream_set_bit_stream(zfp, stream);

    zfp_field_set_type(field, zfp_type_qreal);
    // Add support for handling multi dimensional
    zfp_field_set_size_1d(field, in_blocks->n_values);

    /* Add support for other moddes */
    switch (mem->mode) {
        case 'r':
            zfp_stream_set_rate(zfp, zfp_conf->rate, zfp_type_qreal, zfp_conf->dimensions, zfp_false);
    }

    zfp_stream_set_execution(zfp, zfp_conf->exec);

    zfp_stream_rewind(zfp);

    zfp_field_set_pointer(field, out_block->data);

    zfp_decompress(zfp, field);

    out_block->n_values = in_block->n_values;

    zfp_field_free(field);  
    zfp_stream_close(zfp);
    stream_close(stream);

}