#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "compression.h"

bool rawDataBlock_is_current_block(DecompressedBlock* block, long long int block_idx) {
    return block->used && 
           block->mem_block_index == block_idx;
}

qreal rawDataBlock_get_value(DecompressedBlock* block, long long int index) {
    if (!block->used) {
        printf("Error, cannot fetch data, block not used. Trying internal index: %lli\n", index);
        return 0.0;
    }

    if (block->n_values <= index) {
        printf("Error, cannot fetch index, index out of range: n_values != index, %li != %lli\n", block->n_values, index);
        return 0.0;
    }

    return block->data[index];
}

bool rawDataBlock_set_value(DecompressedBlock* block, long long int index, qreal value) {
    if (!block->used) {
        printf("Error, cannot set data, block not used. Trying interal index: %lli\n", index);
        return false;
    }

    if (block->n_values <= index) {
        printf("Error, cannot fetch index, index out of range: n_values != index, %li != %lli\n", block->n_values, index);
        return false;
    }

    block->data[index] = value;
    return true;
}

void rawDataBlock_dump_to_file(DecompressedBlock *block, FILE *stream) {
    if (!block->used) {
        printf("Missing data, raw data block does not contain any data\n");
        return;
    }

    /* Fire and forget */
    fwrite(block->data, sizeof(*(block->data)), block->n_values, stream);
}

size_t compression_maxSize(CompressionImp *imp) {
    return imp->max_size(imp->config);
}

void compression_compress(CompressionImp *imp,  CompressedBlock *out_block, DecompressedBlock *in_block) {
    imp->compress(imp->config, out_block, in_block);
}

void compression_decompress(CompressionImp *imp, CompressedBlock *in_block, DecompressedBlock *out_block) {
    imp->decompress(imp->config, in_block, out_block);
}

CompressedMemory* compressedMemory_allocate(CompressionConfig conf) {
    CompressedMemory *mem = calloc(1, sizeof(CompressedMemory));

    mem->imp = conf.imp;
    mem->n_blocks = conf.n_blocks;
    mem->values_per_block = conf.values_per_block;

    size_t max_n = compression_maxSize(&mem->imp);

    /* Allocate the number of blocks */
    /* For each block allocate the memory for max_n */
    printf("Creating %li blocks for memory, Max block size %li, total Max size: %lli\n", 
                mem->n_blocks, max_n, (long long int) (mem->n_blocks * max_n));

    mem->blocks = calloc(mem->n_blocks, sizeof(CompressedBlock));
    for (int i = 0; i < mem->n_blocks; i++) {
        mem->blocks[i].n_values = mem->values_per_block;
        mem->blocks[i].max_size = max_n;
        mem->blocks[i].size = 0;
        // if dynamic allocation
        if (conf.use_dynamic_allocation) {
            mem->blocks[i].data = NULL;
        } else {
            mem->blocks[i].data = calloc(max_n, sizeof(char));
        }
    }

    if (conf.use_dynamic_allocation) {
        mem->tmp_block.data = calloc(max_n, sizeof(char));
        mem->tmp_block.max_size = max_n;
        mem->tmp_block.size = 0;
        mem->tmp_block.n_values = 0;
    } else {
        mem->tmp_block.data = NULL;
        mem->tmp_block.max_size = 0;
        mem->tmp_block.size = 0;
        mem->tmp_block.n_values = 0;
    }

    return mem;
}

void compressedMemory_destroy(CompressedMemory *mem) {
    for (int i = 0; i < mem->n_blocks; i++) {
        if (mem->blocks[i].data != NULL) {
            free(mem->blocks[i].data);
        }
    }
    /* Free tmp block for dynamic allocation */
    if (mem->tmp_block.data != NULL) {
        free(mem->tmp_block.data);
    }

    free(mem->blocks);
    free(mem);
}

void compressedMemory_save(CompressedMemory *mem, DecompressedBlock* block) {
    if (!block->used) {
        printf("Error, cannot save uncompressed data, block contains no data.\n");
        return;
    }

    size_t index = block->mem_block_index;
    CompressedBlock *buffer_block = &mem->blocks[index];
    CompressedBlock *out_block =  &mem->blocks[index];

    if (mem->tmp_block.data != NULL) {
        buffer_block = &mem->tmp_block;
    }

    compression_compress(&mem->imp, buffer_block, block);

    /* Handling Dynamic Allocation */
    if (buffer_block != out_block) {
        /* Check if block is allocated already or not */
        if (out_block->data == NULL) {
            /* No memory block exists */
            out_block->data = malloc(buffer_block->size);
        } else if(out_block->size != buffer_block->size) {
            /* Reallocate memory block */
            out_block->data = realloc(out_block->data, buffer_block->size);
        }
        /* Copy all the data from the buffer to the out block */
        memcpy(out_block->data, buffer_block->data, buffer_block->size);
        out_block->size = buffer_block->size;
        out_block->n_values = buffer_block->n_values;
    }

    block->used = false;
    block->n_values = 0;
}


DecompressedBlock* compressedMemory_load(CompressedMemory *mem, size_t index, RawDataBlock* block) {
    int foundIndex = 0;
    if (block->use_double_blocks) {
        foundIndex = -1;
        /* See if loaded blocks contain the data */
        for(int i = 0; i < 2; i++) {
            if(rawDataBlock_is_current_block(&block->decomp_blocks[i], index)) {
                foundIndex = i;
                break;
            }
        }

        /* Check if block not found. Then use the block that is not resently used */
        if (foundIndex == -1) {
            foundIndex = (block->lru_block + 1) % 2;
        }
    }

    DecompressedBlock *out_block = &block->decomp_blocks[foundIndex];

    if (!rawDataBlock_is_current_block(out_block, index)) {
        if (out_block->used) {
            compressedMemory_save(mem, out_block);
        }

        CompressedBlock *in_block = &mem->blocks[index];

        if (in_block->data != NULL) {
            compression_decompress(&mem->imp, in_block, out_block);
        } else {
            memset(out_block->data, 0, out_block->size);
        }

        out_block->n_values = in_block->n_values;
        out_block->size = in_block->n_values * sizeof(*(out_block->data));
        out_block->mem_block_index = index;
        out_block->used = true;
    }

    
    block->lru_block = foundIndex;

    return out_block;
}

void compressedMemory_save_all(CompressedMemory *mem, RawDataBlock* block) {
    for(int i = 0; i < 2; i++) {
        if(block->decomp_blocks[i].used) {
            compressedMemory_save(mem, &block->decomp_blocks[i]);
        }
    }
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
    
    DecompressedBlock *out_block = compressedMemory_load(mem, block_idx, block);

    // Get specific data for interal index in the uncompressed raw data block
    return rawDataBlock_get_value(out_block, internal_idx);
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
    
    DecompressedBlock *out_block = compressedMemory_load(mem, block_idx, block);  

    rawDataBlock_set_value(out_block, internal_idx, value);
}

void compressedMemory_dump_memory_to_file(CompressedMemory *mem, RawDataBlock *block, FILE *stream) {
    for(size_t i = 0; i < mem->n_blocks; i++) {
        DecompressedBlock *out_block = compressedMemory_load(mem, i, block);
        rawDataBlock_dump_to_file(out_block, stream);
    }
}

RawDataBlock* rawDataBlock_allocate(CompressionConfig conf) {
    RawDataBlock* block = calloc(1, sizeof(RawDataBlock));
    size_t data_size = (size_t) (conf.values_per_block * sizeof(qreal));

    printf("Allocating Raw Data block with data size: %li\n", data_size);

    block->lru_block = 1;
    block->use_double_blocks = conf.use_double_blocks;

    block->decomp_blocks[0].data = malloc(data_size);
    block->decomp_blocks[0].size = data_size;
    block->decomp_blocks[0].n_values = 0;
    block->decomp_blocks[0].mem_block_index = 0;
    block->decomp_blocks[0].used = false;
    block->decomp_blocks[1].data = NULL;

    if (conf.use_double_blocks) {
        printf("Using double raw data blocks!\n");
        block->decomp_blocks[1].data = malloc(data_size);
        block->decomp_blocks[1].size = data_size;
        block->decomp_blocks[1].n_values = 0;
        block->decomp_blocks[1].mem_block_index = 0;
        block->decomp_blocks[1].used = false;
    }

    return block;
}

void rawDataBlock_destroy(RawDataBlock* block) {

    for(int i = 0; i < 2; i++) {
        if (block->decomp_blocks[i].data != NULL) {
            free(block->decomp_blocks[i].data);
        }
    }

    free(block);
}