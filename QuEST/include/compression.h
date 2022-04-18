#ifndef COMPRESSION_H
#define COMPRESSION_H

#include <stdbool.h>

#include "QuEST_precision.h"
#include "zfp.h"
#include "zfp-config.h"

#ifdef __cplusplus
extern "C" {
#endif

// Maximum size of a compressed block
#define MAX_VALUES_PER_BLOCK 1024 // floating point numbers per block

enum CompressionTechnique {
    NO_COMPRESSION=0,
    ZFP_COMPRESSION=1,
    FPZIP_COMPRESSION=2,
    FPC_COMPRESSION=3,
};

typedef enum CompressionTechnique Compression;

typedef struct CompressedBlock {
    size_t n_values;  // Number of floating point values
    size_t max_size;  // Maximum byte size
    size_t size;      // Current byte size
    void* data;       // Pointer to data of set byte size
} CompressedBlock;

typedef struct DecompressedBlock {
    size_t n_values;
    size_t size;
    size_t mem_block_index;         // Index of the block in compressed memory
    bool used;
    qreal* data;
} DecompressedBlock;

typedef struct RawDataBlock {
    DecompressedBlock decomp_blocks[2];
    int lru_block;
    bool use_double_blocks;
} RawDataBlock;

typedef struct CompressionImp {
    size_t (*max_size)(void*);
    void (*compress)(void*, CompressedBlock*, DecompressedBlock*);
    void (*decompress)(void*, CompressedBlock*, DecompressedBlock*);
    void *config;
} CompressionImp;

typedef struct CompressedMemory {
    size_t n_blocks;
    size_t values_per_block;        // The number of values per each block e.g. x / values_per_block => block index
    CompressionImp imp;
    CompressedBlock* blocks;        // The compressed blocks of data
    CompressedBlock tmp_block;
    ZFPConfig gpu_zfp_conf;         // Only used for GPU
} CompressedMemory;

typedef struct CompressionConfig {
    CompressionImp imp;
    ZFPConfig gpu_zfp_conf;         // Only used for GPU
    size_t n_blocks;
    size_t values_per_block;
    bool use_dynamic_allocation;
    bool use_double_blocks;
} CompressionConfig;

CompressedMemory* compressedMemory_allocate(CompressionConfig conf);
void compressedMemory_destroy(CompressedMemory *mem);
void compressedMemory_save(CompressedMemory *mem, DecompressedBlock* block);
DecompressedBlock* compressedMemory_load(CompressedMemory *mem, size_t index, RawDataBlock* block);

qreal compressedMemory_get_value(CompressedMemory *mem, RawDataBlock *block, long long int index);
void compressedMemory_set_value(CompressedMemory *mem, RawDataBlock *block, long long int index, qreal value);

void compressedMemory_dump_memory_to_file(CompressedMemory *mem, RawDataBlock *block, FILE *stream);

RawDataBlock* rawDataBlock_allocate(CompressionConfig conf);
void rawDataBlock_destroy(RawDataBlock* block);

#ifdef __cplusplus
}
#endif

#endif