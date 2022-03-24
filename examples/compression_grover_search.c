/** @file 
 * Implements Grover's algorithm for unstructured search,
 * using only X, H and multi-controlled Z gates
 *
 * Compile and run from within the build folder, using:
cmake .. -DUSER_SOURCE=../examples/grovers_search.c \
        -DOUTPUT_EXE=grovers
make
./grovers
 *
 *
 * @author Erik Ljung
 */

# include <stdio.h>
# include <stdlib.h>
# include <math.h>
# include <time.h>
# include <string.h>
# include <unistd.h>
# include <stdbool.h>

#include <stdlib.h>

# include "QuEST.h"
# include "QuEST_precision.h"
# include "zfp-integration.h"
# include "zfp.h"

# include "fpzip-integration.h"
# include "fpzip.h"


/* effect |solElem> -> -|solElem> via a 
 * multi-controlled phase flip gate 
 */
void applyOracle(Qureg qureg, int numQubits, int solElem) {
    
    // apply X to transform |111> into |solElem>
    for (int q=0; q<numQubits; q++)
        if (((solElem >> q) & 1) == 0)
            pauliX(qureg, q);
        
    // effect |111> -> -|111>    
    int ctrls[numQubits];
    for (int q=0; q<numQubits; q++)
        ctrls[q] = q;
    multiControlledPhaseFlip(qureg, ctrls, numQubits);
    
    // apply X to transform |solElem> into |111>
    for (int q=0; q<numQubits; q++)
        if (((solElem >> q) & 1) == 0)
            pauliX(qureg, q);
}



/* apply 2|+><+|-I by transforming into the Hadamard basis 
 * and effecting 2|0><0|-I. We do this, by observing that 
 *   c..cZ = diag{1,..,1,-1} 
 *         = I - 2|1..1><1..1|
 * and hence 
 *   X..X c..cZ X..X = I - 2|0..0><0..0|
 * which differs from the desired 2|0><0|-I state only by 
 * the irrelevant global phase pi
 */
void applyDiffuser(Qureg qureg, int numQubits) {
    
    // apply H to transform |+> into |0>
    for (int q=0; q<numQubits; q++)
        hadamard(qureg, q);

    // apply X to transform |11..1> into |00..0>
    for (int q=0; q<numQubits; q++)
        pauliX(qureg, q);
    
    // effect |11..1> -> -|11..1>
    int ctrls[numQubits];
    for (int q=0; q<numQubits; q++)
        ctrls[q] = q;
    multiControlledPhaseFlip(qureg, ctrls, numQubits);
    
    // apply X to transform |00..0> into |11..1>
    for (int q=0; q<numQubits; q++)
        pauliX(qureg, q);
    
    // apply H to transform |0> into |+>
    for (int q=0; q<numQubits; q++)
        hadamard(qureg, q);
}



void grover_search(int numQubits, QuESTEnv env) {
    
    // choose the system size
    int numElems = (int) pow(2, numQubits);
    int numReps = ceil(M_PI/4 * sqrt(numElems));
    
    printf("numQubits: %d, numElems: %d, numReps: %d\n", 
        numQubits, numElems, numReps);
    
    // randomly choose the element for which to search
    srand(time(NULL));
    int solElem = rand() % numElems;
    
    // prepare |+>
    Qureg qureg = createQureg(numQubits, env);
    initPlusState(qureg);
    
    // apply Grover's algorithm
    for (int r=0; r<numReps; r++) {
        applyOracle(qureg, numQubits, solElem);
        applyDiffuser(qureg, numQubits);
        
        // monitor the probability of the solution state
        printf("[%i] prob of solution |%d> = %g\n", 
           r, solElem, getProbAmp(qureg, solElem));
    }

    // free memory 
    destroyQureg(qureg, env);
    destroyQuESTEnv(env);
}

#define EXIT_FAILURE 1 

static void usage_zfp() {
    fprintf(stderr, "Usage: grover zfp <options>\n");
    fprintf(stderr, "General Options:\n");
    fprintf(stderr, "  -d : use dynamic memory allocation for compressed blocks\n");
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
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "  -q 28 -3 100 100 100 -r 16 : 2x fixed-rate compression of 100x100x100 floats\n");
    fprintf(stderr, "  -q 28 -1 1000000 -r 32 : 2x fixed-rate compression of 1M doubles\n");
    fprintf(stderr, "  -q 28 -2 1000 1000 -p 32 -d : 32-bit precision compression of 1000x1000 doubles with dynamic allocation\n");
    fprintf(stderr, "  -q 28 -1 1000000 -a 1e-9 -d : compression of 1M doubles with < 1e-9 max error with dynamic allocation\n");
    exit(EXIT_FAILURE);
}

static void usage_fpz() {
    fprintf(stderr, "Usage: grover fpzip <options>\n");
    fprintf(stderr, "General Options:\n");
    fprintf(stderr, "  -q <qubits>: number of qubits\n");
    fprintf(stderr, "  -1 <nx> : dimensions for 1D array a[nx]\n");
    fprintf(stderr, "  -2 <nx> <ny> : dimensions for 2D array a[ny][nx]\n");
    fprintf(stderr, "  -3 <nx> <ny> <nz> : dimensions for 3D array a[nz][ny][nx]\n");
    fprintf(stderr, "  -4 <nx> <ny> <nz> <nw> : dimensions for 4D array a[nw][nz][ny][nx]\n");
    fprintf(stderr, "Compression parameters:\n");
    fprintf(stderr, "  -p <precision> : fixed precision (# uncompressed bits per value)\n");
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "  -q 28 -3 100 100 100 -p 16 : 16-bit precision compression of 100x100x100 floats\n");
    fprintf(stderr, "  -q 28 -2 1000 1000 -p 32 : 32-bit precision compression of 1000x1000 floats\n");
    exit(EXIT_FAILURE);
}

void zfp_imp (int argc, char** argv) {
    uint qubits = 0;
    uint dims = 0;
    size_t block_size, nx, ny, nz, nw;
    char mode = 0;
    double tolerance = 0;
    uint precision = 0;
    double rate = 0;
    zfp_exec_policy exec = zfp_exec_serial;
    zfp_type type = zfp_type_qreal;
    bool use_dynamic_allocation = false;

    /* parse command-line arguments */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' || argv[i][2]) { usage_zfp(); }
        switch (argv[i][1]) {
        case '1':
            if (++i == argc || sscanf(argv[i], "%zu", &nx) != 1) { usage_zfp(); }
            block_size = nx;
            ny = nz = nw = 1;
            dims = 1;
            break;
        case '2':
            if (++i == argc || sscanf(argv[i], "%zu", &nx) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &ny) != 1) { usage_zfp(); }
            block_size = nx * ny;
            nz = nw = 1;
            dims = 2;
            break;
        case '3':
            if (++i == argc || sscanf(argv[i], "%zu", &nx) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &ny) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &nz) != 1) { usage_zfp(); }
            block_size = nx * ny * nz;
            nw = 1;
            dims = 3;
            break;
        case '4':
            if (++i == argc || sscanf(argv[i], "%zu", &nx) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &ny) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &nz) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &nw) != 1) { usage_zfp(); }
            block_size = nx * ny * nz * nw;
            dims = 4;
            break;
        case 'a':
            if (++i == argc || sscanf(argv[i], "%lf", &tolerance) != 1) { usage_zfp(); }
            mode = ACCURACY_MODE;
            break;
        case 'p':
            if (++i == argc || sscanf(argv[i], "%u", &precision) != 1) { usage_zfp(); }
            mode = PERCISION_MODE;
            break;
        case 'r':
            if (++i == argc || sscanf(argv[i], "%lf", &rate) != 1) { usage_zfp(); }
            mode = RATE_MODE;
            break;
        case 'R':
            mode = LOSSLESS_MODE;
            break;
        case 'q':
            if (++i == argc || sscanf(argv[i], "%u", &qubits) != 1) { usage_zfp(); }
            break;
        case 'd':
            use_dynamic_allocation = true;
            break;
        default:
            usage_zfp();
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

    printf("nx: %li\n", nx);
    printf("block_size: %li\n", block_size);

    if (!zfpValidateConfig(conf)) {
        usage_zfp();
    } else if (qubits == 0) {
        fprintf(stderr, "Invalid number of qubits\n");
        usage_zfp();
    }

    QuESTEnv env = createQuESTEnvWithZFP(conf, block_size, use_dynamic_allocation);

    grover_search(qubits, env);
}


void fpzip_imp (int argc, char** argv) {
    uint qubits = 0;
    size_t block_size, nx, ny, nz, nw;
    int type = fpzip_type_qreal;
    int precision = 0;

    /* parse command-line arguments */
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-' || argv[i][2]) { usage_fpz(); }
        switch (argv[i][1]) {
        case '1':
            if (++i == argc || sscanf(argv[i], "%zu", &nx) != 1) { usage_fpz(); }
            block_size = nx;
            ny = nz = nw = 1;
            break;
        case '2':
            if (++i == argc || sscanf(argv[i], "%zu", &nx) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &ny) != 1) { usage_fpz(); }
            block_size = nx * ny;
            nz = nw = 1;
            break;
        case '3':
            if (++i == argc || sscanf(argv[i], "%zu", &nx) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &ny) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &nz) != 1) { usage_fpz(); }
            block_size = nx * ny * nz;
            nw = 1;
            break;
        case '4':
            if (++i == argc || sscanf(argv[i], "%zu", &nx) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &ny) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &nz) != 1 ||
                ++i == argc || sscanf(argv[i], "%zu", &nw) != 1) { usage_fpz(); }
            block_size = nx * ny * nz * nw;
            break;
        case 'p':
            if (++i == argc || sscanf(argv[i], "%u", &precision) != 1) { usage_fpz(); }
            break;
        case 'q':
            if (++i == argc || sscanf(argv[i], "%u", &qubits) != 1) { usage_fpz(); }
            break;
        default:
            usage_fpz();
            break;
        }
    }

    FPZIPConfig conf = {
        .nx = nx,
        .ny = ny,
        .nz = nz,
        .nw = nw,
        .precision = precision,
        .type = type
    };

    if (!fpzipValidateConfig(conf)) {
        usage_fpz();
    } else if (qubits == 0) {
        fprintf(stderr, "Invalid number of qubits\n");
        usage_fpz();
    }

    QuESTEnv env = createQuESTEnvWithFPZIP(conf, block_size);

    grover_search(qubits, env);
}

void usage() {
    fprintf(stderr, "Usage: grover <command>\n");
    fprintf(stderr, "Command:\n");
    fprintf(stderr, "  zfp : using zfp compression\n");
    fprintf(stderr, "  fpzip : using fpzip compression\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char** argv)
{
    if (argc < 2) {
        usage();
    }

    printf("Argv[1]: %s\n", argv[1]);

    if (!strncmp(argv[1], "zfp", 4)) {
        zfp_imp(argc-1, ++argv);
    } else if (!strncmp(argv[1], "fpzip", 6)) {
        fpzip_imp(argc-1, ++argv);
    } else {
        fprintf(stderr, "Invalid command");
        usage();
    }

    return 0;
}
