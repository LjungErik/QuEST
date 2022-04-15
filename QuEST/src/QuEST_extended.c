#include "QuEST_extended.h"
#include "compression.h"

qreal getQuregRealValue(Qureg *qureg, long long int index) {
    if (qureg->comp != NO_COMPRESSION) {
        return compressedMemory_get_value(qureg->real_mem, qureg->real_block, index);
    } else {
        return qureg->stateVec.real[index];
    }
}

void setQuregRealValue(Qureg *qureg, long long int index, qreal value) {
    if (qureg->comp != NO_COMPRESSION) {
        compressedMemory_set_value(qureg->real_mem, qureg->real_block, index, value);
    } else {
        qureg->stateVec.real[index] = value;
    }
}

qreal getQuregImagValue(Qureg *qureg, long long int index) {
    if (qureg->comp != NO_COMPRESSION) {
        return compressedMemory_get_value(qureg->imag_mem, qureg->imag_block, index);
    } else {
        return qureg->stateVec.imag[index];
    }
}

void setQuregImagValue(Qureg *qureg, long long int index, qreal value) {
    if (qureg->comp != NO_COMPRESSION) {
        compressedMemory_set_value(qureg->imag_mem, qureg->imag_block, index, value);
    } else {
        qureg->stateVec.imag[index] = value;
    }
}

void dumpQuregStateToFile(Qureg *qureg, char *filename) {
    FILE *out_file = fopen(filename, "w");

    compressedMemory_dump_memory_to_file(qureg->real_mem, qureg->real_block, out_file);
    compressedMemory_dump_memory_to_file(qureg->imag_mem, qureg->imag_block, out_file);

    fclose(out_file);
}