#ifndef ZFP_CONFIG_H
#define ZFP_CONFIG_H

#include "zfp.h"

typedef struct ZFPConfig {
    unsigned int dimensions;
    size_t nx;
    size_t ny;
    size_t nz;
    size_t nw;
    char mode;
    double tolerance;
    uint precision;
    double rate;
    zfp_exec_policy exec;
    zfp_type type;
} ZFPConfig;

#endif