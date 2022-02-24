#include <stdlib.h>

#include "zfp-integration.h"
#include "bitstream.h"

void compressedMemory_allocate(CompressedMemory *mem) {
    zfp_stream* zfp = zfp_stream_open(NULL);
    zfp_field* field = zfp_field_alloc();

    zfp_field_set_type(field, zfp_type_qreal);
    zfp_field_set_size_1d(field, mem->values_per_block); // 1024 block later feature

    switch (mem->mode) {
        case 'r':
            zfp_stream_set_rate(zfp, mem->rate, zfp_type_qreal, mem->dimensions, zfp_false);
    }

    zfp_stream_set_execution(zfp, mem->exec);

    size_t max_n = zfp_stream_maximum_size(zfp, field);

    /* Allocate the number of blocks */
    /* For each block allocate the memory for max_n */
    mem->blocks = calloc(mem->n_blocks, sizeof(CompressedBlock));
    for (int i = 0; i < mem->n_blocks; i++) {
        mem->blocks[i].n_values = mem->values_per_block;
        mem->blocks[i].max_size = max_n;
        mem->blocks[i].size = 0; 
        mem->blocks[i].data = malloc(max_n);
    }

    zfp_field_free(field);  
    zfp_stream_close(zfp);
}

void compressedMemory_destroy(CompressedMemory *mem) {
    for (int i = 0; i < mem->n_blocks; i++) {
        free(mem->blocks[i].data);
    }
    free(mem->blocks);
}

void compressedMemory_save(CompressedMemory *mem, RawDataBlock* block) {
    bitstream* stream;
    zfp_stream* zfp;
    zfp_field* field;

    zfp = zfp_stream_open(NULL); 
    field = zfp_field_alloc();

    zfp_field_set_pointer(field, block->data);
    zfp_field_set_type(field, zfp_type_qreal);
    zfp_field_set_size_1d(field, mem->values_per_block); // Until further

    switch (mem->mode) {
        case 'r':
            zfp_stream_set_rate(zfp, mem->rate, zfp_type_qreal, mem->dimensions, zfp_false);
    }

    zfp_stream_set_execution(zfp, mem->exec);

    size_t index = block->mem_block_index;
    stream = stream_open(mem->blocks[index].data, mem->blocks[index].max_size);

    zfp_stream_set_bit_stream(zfp, stream);

    size_t zfpsize = zfp_compress(zfp, field);
    mem->blocks[index].size = zfpsize;
    block->used = false;

    zfp_field_free(field);  
    zfp_stream_close(zfp);
    stream_close(stream);
}


void compressedMemory_load(CompressedMemory *mem, size_t index, RawDataBlock* block) {
    bitstream* stream;
    zfp_stream* zfp;
    zfp_field* field;

    zfp = zfp_stream_open(NULL); 
    field = zfp_field_alloc();

    stream = stream_open(mem->blocks[index].data, mem->blocks[index].size);
    zfp_stream_set_bit_stream(zfp, stream);

    zfp_field_set_type(field, zfp_type_qreal);
    zfp_field_set_size_1d(field, mem->values_per_block); // Until further

    switch (mem->mode) {
        case 'r':
            zfp_stream_set_rate(zfp, mem->rate, zfp_type_qreal, mem->dimensions, zfp_false);
    }

    zfp_stream_set_execution(zfp, mem->exec);

    zfp_stream_rewind(zfp);

    zfp_field_set_pointer(field, block->data);

    zfp_decompress(zfp, field);

    block->mem_block_index = index;
    block->used = true;

    zfp_field_free(field);  
    zfp_stream_close(zfp);
    stream_close(stream);
}

void rawDataBlock_init(RawDataBlock* block, size_t n_values) {
    size_t data_size = (size_t) (n_values * sizeof(*(block->data)));
    block->data = malloc(data_size);
    block->size = data_size;
    block->used = false;
}

void rawDataBlock_destroy(RawDataBlock* block) {
    free(block->data);
}
