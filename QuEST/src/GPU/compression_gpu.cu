#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "compression.h"
#include "zfp-integration.h"

//static ZFPConfig global_config;

void cudaCopyDataToGPU(void *dst, const void *src, size_t count) {
    printf("Copy data from Host RAM to GPU VRAM, size: %li\n", count);
    cudaMemcpy(dst, src, count, cudaMemcpyHostToDevice);
}

void cudaCopyDataFromGPU(void *dst, const void *src, size_t count) {
    printf("Copy data from Host RAM to GPU VRAM, size: %li\n", count);
    cudaMemcpy(dst, src, count, cudaMemcpyDeviceToHost);
}

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

    qreal ret;

    cudaCopyDataFromGPU(&ret, block->data, sizeof(qreal));

    return ret;
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

    cudaCopyDataToGPU(block->data+index, &value, sizeof(qreal));

    return true;
}

size_t compression_maxSize(ZFPConfig *conf) {
    return zfpMaxSize(conf);
}

void compression_compress(ZFPConfig *conf,  CompressedBlock *out_block, RawDataBlock *in_block) {
    zfpCompress(conf, out_block, in_block);
}

void compression_decompress(ZFPConfig *conf, CompressedBlock *in_block, RawDataBlock *out_block) {
    zfpDecompress(conf, in_block, out_block);
}

CompressedMemory* compressedMemory_allocate(CompressionConfig conf) {
    CompressedMemory *mem = (CompressedMemory*) malloc(sizeof(CompressedMemory));
    //cudaMalloc(&mem, sizeof(CompressedMemory));
    printf("Setting up config data...\n");

    // Need to use special function to copy from RAM to VRAM
    //cudaCopyDataToGPU(&mem->gpu_zfp_conf, &conf.gpu_zfp_conf, sizeof(mem->gpu_zfp_conf));
    //global_config = conf.gpu_zfp_conf;
    //cudaCopyDataToGPU(&mem->n_blocks, &conf.n_blocks, sizeof(mem->n_blocks));
    //cudaCopyDataToGPU(&mem->values_per_block, &conf.values_per_block, sizeof(mem->values_per_block));
    mem->gpu_zfp_conf = conf.gpu_zfp_conf;
    mem->n_blocks = conf.n_blocks;
    mem->values_per_block = conf.values_per_block;

    printf("Setup done\n");

    size_t max_n = compression_maxSize(&mem->gpu_zfp_conf);

    printf("maxSize\n");

    /* Allocate the number of blocks */
    /* For each block allocate the memory for max_n */
    //printf("Creating %li blocks for memory, Max block size %li, total Max size: %lli\n", 
    //            mem->n_blocks, max_n, (long long int) (mem->n_blocks * max_n));

    printf("Allocating VRAM\n");
    mem->blocks = (CompressedBlock *) malloc(mem->n_blocks * sizeof(CompressedBlock));
    //cudaError t = cudaMalloc(&(mem->blocks), (mem->n_blocks * sizeof(CompressedBlock)));
    //printf("Allocation status: %x\n", t);
    for (int i = 0; i < mem->n_blocks; i++) {
        printf("%i - mem block creation\n", i);
        mem->blocks[i].n_values = mem->values_per_block;
        mem->blocks[i].max_size = max_n;
        mem->blocks[i].size = 0;
        // if dynamic allocation
        if (conf.use_dynamic_allocation) {
            mem->blocks[i].data = NULL;
        } else {
            cudaMalloc(&(mem->blocks[i].data), max_n*sizeof(char));
        }
    }

    return mem;
}

void compressedMemory_destroy(CompressedMemory *mem) {
    for (int i = 0; i < mem->n_blocks; i++) {
        if (mem->blocks[i].data != NULL) {
            cudaFree(mem->blocks[i].data);
        }
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
    //printf("Compress block: %li\n", index);
    compression_compress(&mem->gpu_zfp_conf, &mem->blocks[index], block);

    block->used = false;
    block->n_values = 0;
}


void compressedMemory_load(CompressedMemory *mem, size_t index, RawDataBlock* block) {
    if (block->used) {
        compressedMemory_save(mem, block);
    }

    compression_decompress(&mem->gpu_zfp_conf, &mem->blocks[index], block);

    block->mem_block_index = index;
    block->used = true;
}

qreal compressedMemory_get_value(CompressedMemory *mem, RawDataBlock *block, long long int index) {
    long long int block_idx = (index / mem->values_per_block);

    if (block_idx >= mem->n_blocks) {
        printf("Invalid block index, block index out of range, block index: %lli\n", block_idx);
        return 0.0;
    }

    // Calculate local index inside block (index -> internal block index, range. 0-1023) 
    long long int internal_idx = index - (block_idx * mem->values_per_block);
    
    // Check if RawDataBlock is the correct block index
    if (!rawDataBlock_is_current_block(block, block_idx)) {
        // Decompress the specific block
        compressedMemory_load(mem, block_idx, block);
    }  
        
    // Get specific data for interal index in the uncompressed raw data block
    return rawDataBlock_get_value(block, internal_idx);
}

void compressedMemory_set_value(CompressedMemory *mem, RawDataBlock *block, long long int index, qreal value) {
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
        compressedMemory_load(mem, block_idx, block);
    }  

    rawDataBlock_set_value(block, internal_idx, value);
}

RawDataBlock* rawDataBlock_allocate(CompressionConfig conf) {
    RawDataBlock* block = (RawDataBlock*) malloc(sizeof(RawDataBlock));
    //cudaMalloc(&block, sizeof(RawDataBlock));
    size_t data_size = (size_t) (conf.values_per_block * sizeof(*(block->data)));

    printf("Allocating Raw Data block with data size: %li\n", data_size);

    if (conf.use_dynamic_allocation) {
        size_t max_n = compression_maxSize(&conf.gpu_zfp_conf);
        cudaMalloc(&(block->tmp_storage), (max_n * sizeof(char)));
        block->tmp_max_size = max_n;
    } else {
        block->tmp_storage = NULL;
        block->tmp_max_size = 0;
    }

    cudaMalloc(&(block->data), data_size);
    block->size = data_size;
    block->n_values = 0;
    block->mem_block_index = 0;
    block->used = false;

    return block;
}

void rawDataBlock_destroy(RawDataBlock* block) {
    if (block->tmp_storage != NULL) {
        cudaFree(block->tmp_storage);
    }

    cudaFree(block->data);
    free(block);
}

void rawDataBlock_dump_to_file(RawDataBlock *block, FILE *stream) {
    if (!block->used) {
        printf("Missing data, raw data block does not contain any data\n");
        return;
    }

    /* Fire and forget */
    //fwrite(block->data, sizeof(*(block->data)), block->n_values, stream);
}

void compressedMemory_dump_memory_to_file(CompressedMemory *mem, RawDataBlock *block, FILE *stream) {
    /* if in use, save current state */
    if (block->used) {
        // Compress the existing block and save to memory
        compressedMemory_save(mem, block);
    }

    for(size_t i = 0; i < mem->n_blocks; i++) {
        block->used = false;
        compressedMemory_load(mem, i, block);
        rawDataBlock_dump_to_file(block, stream);
    }
}