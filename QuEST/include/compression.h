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

typedef struct CompressionImp_vtable_ {
    const size_t (*max_size)(void*, RawDataBlock*);
    const void (*compress)(void*, CompressedBlock*, RawDataBlock*);
    const void (*decompress)(void*, CompressedBlock*, RawDataBlock*);
} CompressionImp_vtable_;

typedef struct CompressionImp {
    CompressionImp_vtable_ *vtable_;
    void *config;
} CompressionImp;

typedef struct CompressedBlock {
    size_t n_values;  // Number of floating point values
    size_t max_size;  // Maximum byte size
    size_t size;      // Current byte size
    void* data;       // Pointer to data of set byte size, DYNAMICALLY ALLOCATE
} CompressedBlock;

typedef struct CompressedMemory {
    size_t n_blocks;
    size_t values_per_block;        // The number of values per each block e.g. x / values_per_block => block index
    CompressionImp imp;
    CompressedBlock* blocks;        // The compressed blocks of data
} CompressedMemory;

typedef struct RawDataBlock {
    size_t n_values;
    size_t size;
    size_t mem_block_index;         // Index of the block in compressed memory
    bool used;
    qreal* data;
} RawDataBlock;

CompressedMemory* compressedMemory_allocate(CompressionConfig *config);
void compressedMemory_destroy(CompressedMemory *mem);
bool compressedMemory_save(CompressedMemory *mem, RawDataBlock* block); // Compressing
bool compressedMemory_load(CompressedMemory *mem, size_t index, RawDataBlock* block); // Decompressing

qreal compressedMemory_get_value(CompressedMemory *mem, RawDataBlock *block, long long int index);
void compressedMemory_set_value(CompressedMemory *mem, RawDataBlock *block, long long int index, qreal value);

RawDataBlock* rawDataBlock_allocate(size_t n_values);
void rawDataBlock_destroy(RawDataBlock* block);
bool rawDataBlock_is_current_block(RawDataBlock* block, long long int block_idx);

#endif