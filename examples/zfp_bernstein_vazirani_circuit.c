/** @file 
 * Implements the Bernstien--Vazirani circuit
 *
 * @author Tyson Jones
 */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <unistd.h>

# include "QuEST.h"
# include "QuEST_precision.h"
# include "zfp-integration.h"
# include "zfp.h"

#define EXIT_FAILURE 1 

static void usage() {
    fprintf(stderr, "Usage: bernvaz <options>\n");
    fprintf(stderr, "General Options:\n");
    fprintf(stderr, "  -q <qubits>: number of qubits\n");
    fprintf(stderr, "  -1 <nx> : dimensions for 1D array a[nx]\n");
    fprintf(stderr, "  -2 <nx> <ny> : dimensions for 2D array a[ny][nx]\n");
    fprintf(stderr, "  -3 <nx> <ny> <nz> : dimensions for 3D array a[nz][ny][nx]\n");
    fprintf(stderr, "  -4 <nx> <ny> <nz> <nw> : dimensions for 4D array a[nw][nz][ny][nx]\n");
    fprintf(stderr, "Compression parameters:\n");
    fprintf(stderr, "  -R : reversible (lossless) compression\n");
    fprintf(stderr, "  -r <rate> : fixed rate (# compressed bits per floating-point value)\n");
    fprintf(stderr, "  -p <precision> : fixed precision (# uncompressed bits per value)\n");
    fprintf(stderr, "  -a <tolerance> : fixed accuracy (absolute error tolerance)\n");
    // fprintf(stderr, "  -c <minbits> <maxbits> <maxprec> <minexp> : advanced usage\n");
    // fprintf(stderr, "      minbits : min # bits per 4^d values in d dimensions\n");
    // fprintf(stderr, "      maxbits : max # bits per 4^d values in d dimensions (0 for unlimited)\n");
    // fprintf(stderr, "      maxprec : max # bits of precision per value (0 for full)\n");
    // fprintf(stderr, "      minexp : min bit plane # coded (-1074 for all bit planes)\n");
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "  -q 28 -3 100 100 100 -r 16 : 2x fixed-rate compression of 100x100x100 floats\n");
    fprintf(stderr, "  -q 28 -1 1000000 -r 32 : 2x fixed-rate compression of 1M doubles\n");
    fprintf(stderr, "  -q 28 -2 1000 1000 -p 32 : 32-bit precision compression of 1000x1000 doubles\n");
    fprintf(stderr, "  -q 28 -1 1000000 -a 1e-9 : compression of 1M doubles with < 1e-9 max error\n");
    exit(EXIT_FAILURE);
}

void bernstein_vazirani(int numQubits, size_t block_size, ZFPConfig conf) {
    /* 	
     * PREPARE QuEST
     */

    // model parameters
    int secretNum = pow(2,4) + 1;

    // prepare QuEST
    QuESTEnv env = createQuESTEnvWithZFP(conf, 1024);

    // create qureg; let zeroth qubit be ancilla
    Qureg qureg = createQureg(numQubits, env);
    initZeroState(qureg);

    /* 
     * APPLY ALGORITHM
     */

    // NOT the ancilla
    pauliX(qureg, 0);

    // CNOT secretNum bits with ancilla
    int bits = secretNum;
    int bit;
    for (int qb=1; qb < numQubits; qb++) {
        bit = bits % 2;
        bits /= 2;
        if (bit)
            controlledNot(qureg, 0, qb);
    }


    /* 	
     * VERIFY FINAL STATE
     */

    // calculate prob of solution state
    double successProb = 1.0;
    bits = secretNum;
    for (int qb=1; qb < numQubits; qb++) {
        bit = bits % 2;
        bits /= 2;
        successProb *= calcProbOfOutcome(qureg, qb, bit);
    }

    printf("solution reached with probability ");
    printf("%f", successProb);
    printf("\n");

    //reportStateToScreen(qureg, env, 0);


    /*
     * FREE MEMORY
     */

    destroyQureg(qureg, env); 
    destroyQuESTEnv(env);
}

int main (int argc, char** argv) {
    uint qubits = 0;
    uint dims = 0;
    size_t block_size, nx, ny, nz, nw;
    char mode = 0;
    double tolerance = 0;
    uint precision = 0;
    double rate = 0;
    zfp_exec_policy exec = zfp_exec_serial;
    zfp_type type = zfp_type_qreal;

    /* parse command-line arguments */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' || argv[i][2]) { usage(); }
        switch (argv[i][1]) {
        case '1':
            if (++i == argc || sscanf(argv[i], "%zu", &nx) != 1) { usage(); }
            block_size = nx;
            ny = nz = nw = 1;
            dims = 1;
            break;
        case '2':
            if (++i == argc || sscanf(argv[i], "%zu", &nx) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &ny) != 1) { usage(); }
            block_size = nx * ny;
            nz = nw = 1;
            dims = 2;
            break;
        case '3':
            if (++i == argc || sscanf(argv[i], "%zu", &nx) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &ny) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &nz) != 1) { usage(); }
            block_size = nx * ny * nz;
            nw = 1;
            dims = 3;
            break;
        case '4':
            if (++i == argc || sscanf(argv[i], "%zu", &nx) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &ny) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &nz) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &nw) != 1) { usage(); }
            block_size = nx * ny * nz * nw;
            dims = 4;
            break;
        case 'a':
            if (++i == argc || sscanf(argv[i], "%lf", &tolerance) != 1) { usage(); }
            mode = ACCURACY_MODE;
            break;
        // case 'c':
        //     if (++i == argc || sscanf(argv[i], "%u", &minbits) != 1 ||
        //         ++i == argc || sscanf(argv[i], "%u", &maxbits) != 1 ||
        //         ++i == argc || sscanf(argv[i], "%u", &maxprec) != 1 ||
        //         ++i == argc || sscanf(argv[i], "%d", &minexp) != 1) { usage(); }
        //     mode = 'c';
        //     break;
        // case 'h':
        //     header = zfp_true;
        //     break;
        case 'p':
            if (++i == argc || sscanf(argv[i], "%u", &precision) != 1) { usage(); }
            mode = PERCISION_MODE;
            break;
        case 'r':
            if (++i == argc || sscanf(argv[i], "%lf", &rate) != 1) { usage(); }
            mode = RATE_MODE;
            break;
        case 'R':
            mode = LOSSLESS_MODE;
            break;
        case 'q':
            if (++i == argc || sscanf(argv[i], "%u", &qubits) != 1) { usage(); }
            break;
        default:
            usage();
            break;
        }
    }

    ZFPConfig conf = {
        .dimensions = dims,
        .nx = nx,
        .ny = ny,
        .nz = nz,
        .nw = nw,
        .mode = mode,
        .tolerance = tolerance,
        .precision = precision,
        .rate = rate,
        .exec = zfp_exec_serial,
        .type = zfp_type_qreal,
    };

    if (!zfpValidateConfig(conf)) {
        usage();
    } else if (qubits == 0) {
        fprintf(stderr, "Invalid number of qubits\n");
        usage();
    }

    bernstein_vazirani(qubits, block_size, conf);

    return 0;
}
