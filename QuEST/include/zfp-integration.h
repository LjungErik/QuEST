#ifndef ZFP_INTEGRATION_H
#define ZFP_INTEGRATION_H

#include "QuEST_precision.h"
#include "zfp.h"

#define bool char
#define true 1
#define false 0

// Maximum size of a compressed block
#define MAX_VALUES_PER_BLOCK 1024 // floating point numbers per block

typedef struct CompressedBlock {
    size_t n_values;  // Number of floating point values
    size_t max_size;  // Maximum byte size
    size_t size;      // Current byte size
    void* data;       // Pointer to data of set byte size, DYNAMICALLY ALLOCATE
} CompressedBlock;

typedef struct CompressedMemory {
    size_t n_blocks;
    size_t values_per_block;        // The number of values per each block e.g. x / values_per_block => block index
    unsigned int dimensions;
    char mode;
    double rate;
    zfp_exec_policy exec;
    CompressedBlock* blocks;        // The compressed blocks of data
} CompressedMemory;

typedef struct RawDataBlock {
    size_t n_values;
    size_t size;
    size_t mem_block_index;         // Index of the block in compressed memory
    bool used;
    qreal* data;
} RawDataBlock;

void compressedMemory_allocate(CompressedMemory *mem);
void compressedMemory_save(CompressedMemory *mem, RawDataBlock* block); // Compressing
void compressedMemory_load(CompressedMemory *mem, size_t index, RawDataBlock* block); // Decompressing
void compressedMemory_destroy(CompressedMemory *mem);

void rawDataBlock_init(RawDataBlock* block, size_t n_values);
void rawDataBlock_destroy(RawDataBlock* block);

#endif