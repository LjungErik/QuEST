#include "QuEST_extended.h"
#include "zfp-integration.h"

qreal getQuregRealValue(Qureg *qureg, long long int index) {
    if (qureg->comp == ZFP_COMPRESSION) {
        return compressedMemory_get_value(qureg->real_mem, qureg->real_block, index);
    } else {
        return qureg->stateVec.real[index];
    }
}

void setQuregRealValue(Qureg *qureg, long long int index, qreal value) {
    if (qureg->comp == ZFP_COMPRESSION) {
        compressedMemory_set_value(qureg->real_mem, qureg->real_block, index, value);
    } else {
        qureg->stateVec.real[index] = value;
    }
}

qreal getQuregImagValue(Qureg *qureg, long long int index) {
    if (qureg->comp == ZFP_COMPRESSION) {
        return compressedMemory_get_value(qureg->imag_mem, qureg->imag_block, index);
    } else {
        return qureg->stateVec.imag[index];
    }
}

void setQuregImagValue(Qureg *qureg, long long int index, qreal value) {
    if (qureg->comp == ZFP_COMPRESSION) {
        compressedMemory_set_value(qureg->imag_mem, qureg->imag_block, index, value);
    } else {
        qureg->stateVec.imag[index] = value;
    }
}