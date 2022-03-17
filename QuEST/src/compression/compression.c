#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "compression.h"

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

size_t compression_maxSize(CompressionImp *imp) {
    return imp->max_size(imp->config);
}

void compression_compress(CompressionImp *imp,  CompressedBlock *out_block, RawDataBlock *in_block) {
    imp->compress(imp->config, out_block, in_block);
}

void compression_decompress(CompressionImp *imp, CompressedBlock *in_block, RawDataBlock *out_block) {
    imp->decompress(imp->config, in_block, out_block);
}

CompressedMemory* compressedMemory_allocate(CompressionImp imp) {
    CompressedMemory *mem = calloc(1, sizeof(CompressedMemory));

    mem->imp = imp;

    size_t max_n = compression_maxSize(&mem->imp);

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

    return mem;
}

void compressedMemory_destroy(CompressedMemory *mem) {
    for (int i = 0; i < mem->n_blocks; i++) {
        free(mem->blocks[i].data);
    }
    free(mem->blocks);
    free(mem);
}

void compressedMemory_save(CompressedMemory *mem, RawDataBlock* block) {
    if (!block->used) {
        printf("Error, cannot save uncompressed data, block contains no data.\n");
        return;
    }

    size_t index = block->mem_block_index;
    compression_compress(&mem->imp, &mem->blocks[index], block);

    block->used = false;
    block->n_values = 0;
}


void compressedMemory_load(CompressedMemory *mem, size_t index, RawDataBlock* block) {
    if (block->used) {
        printf("Error, cannot load compressed data, block still in use. Existing block index: %li\n", block->mem_block_index);
        return;
    }

    compression_decompress(&mem->imp, &mem->blocks[index], block);

    block->mem_block_index = index;
    block->used = true;
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