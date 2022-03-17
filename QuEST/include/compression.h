#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <stdbool.h>

#include "QuEST_precision.h"
#include "zfp.h"

// Maximum size of a compressed block
#define MAX_VALUES_PER_BLOCK 1024 // floating point numbers per block

enum CompressionTechnique {
    NO_COMPRESSION=0,
    ZFP_COMPRESSION=1
};

typedef enum CompressionTechnique Compression;

typedef struct CompressedBlock {
    size_t n_values;  // Number of floating point values
    size_t max_size;  // Maximum byte size
    size_t size;      // Current byte size
    void* data;       // Pointer to data of set byte size, DYNAMICALLY ALLOCATE
} CompressedBlock;

typedef struct RawDataBlock {
    size_t n_values;
    size_t size;
    size_t mem_block_index;         // Index of the block in compressed memory
    bool used;
    qreal* data;
} RawDataBlock;

typedef struct CompressionImp {
    size_t (*max_size)(void*);
    void (*compress)(void*, CompressedBlock*, RawDataBlock*);
    void (*decompress)(void*, CompressedBlock*, RawDataBlock*);
    void *config;
} CompressionImp;

typedef struct CompressedMemory {
    size_t n_blocks;
    size_t values_per_block;        // The number of values per each block e.g. x / values_per_block => block index
    CompressionImp imp;
    CompressedBlock* blocks;        // The compressed blocks of data
} CompressedMemory;

typedef struct CompressionConfig {
    CompressionImp imp;
    size_t n_blocks;
    size_t values_per_block;
} CompressionConfig;

size_t compression_maxSize(CompressionImp *imp);
void compression_compress(CompressionImp *imp,  CompressedBlock *out_block, RawDataBlock *in_block);
void compression_decompress(CompressionImp *imp, CompressedBlock *in_block, RawDataBlock *out_block);

CompressedMemory* compressedMemory_allocate(CompressionConfig conf);
void compressedMemory_destroy(CompressedMemory *mem);
void compressedMemory_save(CompressedMemory *mem, RawDataBlock* block); // Compressing
void compressedMemory_load(CompressedMemory *mem, size_t index, RawDataBlock* block); // Decompressing

qreal compressedMemory_get_value(CompressedMemory *mem, RawDataBlock *block, long long int index);
void compressedMemory_set_value(CompressedMemory *mem, RawDataBlock *block, long long int index, qreal value);

RawDataBlock* rawDataBlock_allocate(size_t n_values);
void rawDataBlock_destroy(RawDataBlock* block);
bool rawDataBlock_is_current_block(RawDataBlock* block, long long int block_idx);

#endif