#ifndef FPC_H
#define FPC_H

typedef struct FPC_MEM {
    void *ptr;
    size_t pos;
    size_t length;
} FPC_MEM;

size_t fpc_compress(long predsizem1, FPC_MEM *input, FPC_MEM *output);
void fpc_decompress(FPC_MEM *input, FPC_MEM *output);

#endif