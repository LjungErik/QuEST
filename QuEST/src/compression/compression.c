#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "compression.h"
#include "bitstream.h"

bool rawDataBlock_is_current_block(RawDataBlock* block, long long int block_idx) {
    return block->used && 
           block->mem_block_index == block_idx;
}

qreal rawDataBlock_get_value(RawDataBlock* block, long long int index) {
    if (!block->used) {
        printf("Error, cannot fetch data, block not used. Trying internal index: %lli\n", index);
        return 0.0;
    }

    if (block->n_values <= index) {
        printf("Error, cannot fetch index, index out of range");
        return 0.0;
    }

    return block->data[index];
}

bool rawDataBlock_set_value(RawDataBlock* block, long long int index, qreal value) {
    if (!block->used) {
        printf("Error, cannot set data, block not used. Trying interal index: %lli\n", index);
        return false;
    }

    if (block->n_values <= index) {
        printf("Error, cannot fetch index, index out of range");
        return false;
    }

    block->data[index] = value;
    return true;
}

CompressedMemory* compressedMemory_allocate(CompressionConfig *conf) {
    CompressedMemory *mem = calloc(1, sizeof(CompressedMemory));
    zfp_stream* zfp = zfp_stream_open(NULL);
    zfp_field* field = zfp_field_alloc();

    mem->n_blocks = conf->n_blocks;
    mem->values_per_block = conf->values_per_block;
    mem->dimensions = conf->dimensions;
    mem->mode = conf->mode;
    mem->rate = conf->rate;
    mem->exec = conf->exec;

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
    printf("Allocating %li blocks for memory with block size %li, total size: %lli\n", 
                mem->n_blocks, max_n, (long long int) (mem->n_blocks * max_n));

    mem->blocks = calloc(mem->n_blocks, sizeof(CompressedBlock));
    for (int i = 0; i < mem->n_blocks; i++) {
        mem->blocks[i].n_values = mem->values_per_block;
        mem->blocks[i].max_size = max_n;
        mem->blocks[i].size = 0; 
        mem->blocks[i].data = calloc(max_n, sizeof(char));
    }

    zfp_field_free(field);
    zfp_stream_close(zfp);

    return mem;
}

void compressedMemory_destroy(CompressedMemory *mem) {
    for (int i = 0; i < mem->n_blocks; i++) {
        free(mem->blocks[i].data);
    }
    free(mem->blocks);
    free(mem);
}

bool compressedMemory_save(CompressedMemory *mem, RawDataBlock* block) {
    if (!block->used) {
        printf("Error, cannot save uncompressed data, block contains no data.\n");
        return false;
    }

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
    block->n_values = 0;

    zfp_field_free(field);  
    zfp_stream_close(zfp);
    stream_close(stream);

    return true;
}


bool compressedMemory_load(CompressedMemory *mem, size_t index, RawDataBlock* block) {
    if (block->used) {
        printf("Error, cannot load compressed data, block still in use. Existing block index: %li\n", block->mem_block_index);
        return false;
    }
    
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
    block->n_values = mem->blocks[index].n_values;
    block->used = true;

    //printf("## Loaded compressed data into memory, index: %li\n", index);

    zfp_field_free(field);  
    zfp_stream_close(zfp);
    stream_close(stream);

    return true;
}

qreal compressedMemory_get_value(CompressedMemory* mem, RawDataBlock* block, long long int index) {
    // Calculate which block index should be in (index -> block index)
    long long int block_idx = (index / mem->values_per_block);

    if (block_idx >= mem->n_blocks) {
        printf("Invalid block index, block index out of range, block index: %lli\n", block_idx);
        return 0.0;
    }

    // Calculate local index inside block (index -> internal block index, range. 0-1023) 
    long long int internal_idx = index - (block_idx * mem->values_per_block);
    
    // Check if RawDataBlock is the correct block index
    if (!rawDataBlock_is_current_block(block, block_idx)) {
        if (block->used) {
            //printf("Saving block: %li\n", block->mem_block_index);
            // Compress the existing block and save to memory
            compressedMemory_save(mem, block);
        }
        // Decompress the specific block
        compressedMemory_load(mem, block_idx, block);
    }  
        
    // Get specific data for interal index in the uncompressed raw data block
    return rawDataBlock_get_value(block, internal_idx);
}

void compressedMemory_set_value(CompressedMemory* mem, RawDataBlock* block, long long int index, qreal value) {
    // Calculate which block index should be in (index -> block index)
    long long int block_idx = (index / mem->values_per_block);

    if (block_idx >= mem->n_blocks) {
        printf("Invalid block index, block index out of range, block index: %lli\n", block_idx);
        return;
    }

    // Calculate local index inside block (index -> internal block index, range. 0-1023) 
    long long int internal_idx = index - (block_idx * mem->values_per_block);
    
    //printf("Setting block: %lli, internal index: %lli, value: %f\n", block_idx, internal_idx, value);

    // Check if RawDataBlock is the correct block index
    if (!rawDataBlock_is_current_block(block, block_idx)) {
        if (block->used) {
            //printf("Saving block: %li\n", block->mem_block_index);
            // Compress the existing block and save to memory
            compressedMemory_save(mem, block);
        }
        // Decompress the specific block
        compressedMemory_load(mem, block_idx, block);
    }  
        
    // Get specific data for interal index in the uncompressed raw data block
    rawDataBlock_set_value(block, internal_idx, value);
}

RawDataBlock* rawDataBlock_allocate(size_t n_values) {
    RawDataBlock* block = calloc(1, sizeof(RawDataBlock));
    size_t data_size = (size_t) (n_values * sizeof(*(block->data)));

    printf("Allocating Raw Data block with data size: %li\n", data_size);

    block->data = malloc(data_size);
    block->size = data_size;
    block->n_values = 0;
    block->mem_block_index = 0;
    block->used = false;

    return block;
}

void rawDataBlock_destroy(RawDataBlock* block) {
    free(block->data);
    free(block);
}