
import os
from tqdm import tqdm
from progress.bar import Bar
import struct

# Binary locations
standard_quest = "../../../../quest_original/QuEST/build/demo"
compression_quest = "../build/grover"

# In files
original_data_path = '../../../../fidelity_test_vectors/QuEST_Test_Bench/TestResults/original/'
zfp_data_path = '../../../../fidelity_test_vectors/QuEST_Test_Bench/TestResults/zfp/'
fpzip_data_path = '../../../../fidelity_test_vectors/QuEST_Test_Bench/TestResults/fpzip/'

# Out files
zfp_out = 'fidelity_tests/zfp'
fpzip_out = 'fidelity_tests/fpzip'

TEST_CASES = {
    "original": {
        10: {"b": 0},
        11: {"b": 0},
        12: {"b": 0},
        13: {"b": 0},
        14: {"b": 0},
        15: {"b": 0},
        16: {"b": 0},
        17: {"b": 0},
        18: {"b": 0}
    },
    "fpzip": {
        10: {
            "b": [32, 64, 128, 256],
            "params": [
                ["-p", "64"],
            ]
        },
        11: {
            "b": [64, 128, 256, 512],
            "params": [
                ["-p", "64"],
            ]
        },
        12: {
            "b": [128, 256, 512, 1024],
            "params": [
                ["-p", "64"],
            ]
        },
        13: {
            "b": [256, 512,1024,2048],
            "params": [

                ["-p", "64"],
            ]
        },
        14: {
            "b": [512,1024,2048,4096],
            "params": [
                ["-p", "64"],
            ]
        },
        15: {
            "b": [1024,2048,4096,8192],
            "params": [
                ["-p", "64"],
            ]
        },
        16: {
            "b": [32,64,128,256,512,1024,2048, 4096, 8192, 16384],
            "params": [
                ["-p", "64"],
            ]
        },
        17: {
            "b": [4096, 8192, 16384, 32768],
            "params": [
                ["-p", "64"],
            ]
        },
        18: {
            "b": [8192, 16384, 32768, 32768],
            "params": [
                ["-p", "64"],
            ]
        },
    },
    "zfp": {
        10: {
            "b": [32, 64, 128, 256],
            "params": [
                ["-r", "16"],
                ["-r", "32"],
                ["-p", "16"],
                ["-p", "32"],
                ["-r", "16", "-d"],
                ["-r", "32", "-d"],
                ["-p", "16", "-d"],
                ["-p", "32", "-d"],
                ["-a", "1e-4"], 
                ["-a", "1e-6"], 
                ["-a", "1e-4", "-d"], 
                ["-a", "1e-6", "-d"]
            ]
        },
        11: {
            "b": [64, 128, 256, 512],
            "params": [
                ["-r", "16"],
                ["-r", "32"],
                ["-p", "16"],
                ["-p", "32"],
                ["-r", "16", "-d"],
                ["-r", "32", "-d"],
                ["-p", "16", "-d"],
                ["-p", "32", "-d"],
                ["-a", "1e-4"], 
                ["-a", "1e-6"], 
                ["-a", "1e-4", "-d"], 
                ["-a", "1e-6", "-d"]
            ]
        },
        12: {
            "b": [128, 256, 512, 1024],
            "params": [
                ["-r", "16"],
                ["-r", "32"],
                ["-p", "16"],
                ["-p", "32"],
                ["-r", "16", "-d"],
                ["-r", "32", "-d"],
                ["-p", "16", "-d"],
                ["-p", "32", "-d"],
                ["-a", "1e-4"], 
                ["-a", "1e-6"], 
                ["-a", "1e-4", "-d"], 
                ["-a", "1e-6", "-d"]
            ]
        },
        13: {
            "b": [256, 512, 1024, 2048],
            "params": [
                ["-r", "16"],
                ["-r", "32"],
                ["-p", "16"],
                ["-p", "32"],
                ["-r", "16", "-d"],
                ["-r", "32", "-d"],
                ["-p", "16", "-d"],
                ["-p", "32", "-d"],
                ["-a", "1e-4"], 
                ["-a", "1e-6"], 
                ["-a", "1e-4", "-d"], 
                ["-a", "1e-6", "-d"]
            ]
        },
        14: {
            "b": [512, 1024, 2048, 4096],
            "params": [
                ["-r", "16"],
                ["-r", "32"],
                ["-p", "16"],
                ["-p", "32"],
                ["-r", "16", "-d"],
                ["-r", "32", "-d"],
                ["-p", "16", "-d"],
                ["-p", "32", "-d"],
                ["-a", "1e-4"], 
                ["-a", "1e-6"], 
                ["-a", "1e-4", "-d"], 
                ["-a", "1e-6", "-d"]
            ]
        },
        15: {
            "b": [1024, 2048, 4096, 8192],
            "params": [
                ["-r", "16"],
                ["-r", "32"],
                ["-p", "16"],
                ["-p", "32"],
                ["-r", "16", "-d"],
                ["-r", "32", "-d"],
                ["-p", "16", "-d"],
                ["-p", "32", "-d"],
                ["-a", "1e-4"], 
                ["-a", "1e-6"], 
                ["-a", "1e-4", "-d"], 
                ["-a", "1e-6", "-d"]
            ]
        },
        16: {
            "b": [32,64,128,256,512,1024,2048, 4096, 8192, 16384],
            "params": [
                ["-r", "16"],
                ["-r", "32"],
                ["-p", "16"],
                ["-p", "32"],
                ["-r", "16", "-d"],
                ["-r", "32", "-d"],
                ["-p", "16", "-d"],
                ["-p", "32", "-d"],
                ["-a", "1e-4"], 
                ["-a", "1e-6"], 
                ["-a", "1e-4", "-d"], 
                ["-a", "1e-6", "-d"]
            ]
        },
        17: {
            "b": [4096, 8192, 16384, 32768],
            "params": [
                ["-r", "16"],
                ["-r", "32"],
                ["-p", "16"],
                ["-p", "32"],
                ["-r", "16", "-d"],
                ["-r", "32", "-d"],
                ["-p", "16", "-d"],
                ["-p", "32", "-d"],
                ["-a", "1e-4"], 
                ["-a", "1e-6"], 
                ["-a", "1e-4", "-d"], 
                ["-a", "1e-6", "-d"]
            ]
        },
        18: {
            "b": [8192, 16384, 32768, 65536],
            "params": [
                ["-r", "16"],
                ["-r", "32"],
                ["-p", "16"],
                ["-p", "32"],
                ["-r", "16", "-d"],
                ["-r", "32", "-d"],
                ["-p", "16", "-d"],
                ["-p", "32", "-d"],
                ["-a", "1e-4"], 
                ["-a", "1e-6"], 
                ["-a", "1e-4", "-d"], 
                ["-a", "1e-6", "-d"]
            ]
        },
    },
}


## Calculates the difference in the metrics contained in file1 and file2
## @param1  The first file
## @param2  The second file
## @param3  Number of values to be read in respective file
## @return  Tuple containing the maximum difference and average difference
def calc_diff_metrics(file1, file2, nr_values):

    tot_diff = 0.0
    max_diff = 0.0

    with Bar("Processing", max=nr_values) as bar:
        f1 = open(file1, 'rb')
        f2 = open(file2, 'rb')
        val1arr = []
        val2arr = []
        for i in range(nr_values):
            [val1] = struct.unpack('d', f1.read(8))
            [val2] = struct.unpack('d', f2.read(8))

            val1arr.append(val1)
            val2arr.append(val2)
        
            bar.next()
        bar.finish()
        f1.close()
        f2.close()
    res = []
    for i in range(len(val1arr)):
        res.append(abs(val1arr[i] - val2arr[i]))
    return res
    

## @param1  Tuple of real and imaginary arrays containing the first state-vector
## @param2  Tuple of real and imaginary arrays containing the seconds state-vector
## Input format:  Tuple(array[real], array[imag])
def calc_fidelity(bra, ket):
    
    if(len(bra[0]) != len(ket[0])):
        print("Invalid sizing of vectors! Both must be of same size.")
    
    
    numAmps = len(bra[0])
    braVecReal = bra[0]
    braVecImag = bra[1]
    ketVecReal = ket[0]
    ketVecImag = ket[1]
    
    #braRe, braIm, ketRe, ketIm, innerProdReal, innerProdImag
    innerProdReal = 0
    innerProdImag = 0
    for index in range(numAmps):
        braRe = braVecReal[index]
        braIm = braVecImag[index]
        ketRe = ketVecReal[index]
        ketIm = ketVecImag[index]
        
        innerProdReal += braRe*ketRe + braIm*ketIm
        innerProdImag += braRe*ketIm - braIm*ketRe
    
    
    
    innerProd = (innerProdReal, innerProdImag)  # (Real, Imaginary) - tuple
    
    return pow(innerProd[0], 2) + pow(innerProd[1], 2)

## Opens a binary and produces an array of the data inside
## @return  The array read from binary
def binary_to_array(file1, nr_values):
    f1 = open(file1, 'rb')
    val1arr = []
    vary = []

    for i in tqdm(range(nr_values)):
        [val1] = struct.unpack('d', f1.read(8))
        val1arr.append(val1)
        vary.append(i)
    f1.close()

    return val1arr

## Needs to be modified
def run_one_fidelity_test(qubits, block_size, param, comp_alg):

    # Run default grover
    os.system(standard_quest + f" {qubits}")
    grover_no_comp = binary_to_array('grover_statevec_dump.data', pow(2, qubits +1))
    

    # Run zfp grover
    os.system(compression_quest + f' {comp_alg} -q {qubits}, -1 {block_size} {param[0]} {param[1]} -z');
    grover_comp = binary_to_array('grover-search_dump.data', pow(2, qubits +1))

    # Run fidelity calculation
    res = calc_fidelity((grover_comp[:len(grover_comp)//2], grover_comp[len(grover_comp)//2:]), (grover_no_comp[:len(grover_no_comp)//2], grover_no_comp[len(grover_no_comp)//2:]))
    innerProdMag = pow(res[0], 2) + pow(res[1], 2)
    print(f"Result: {innerProdMag}")

## Dump the fidelities to the specified path
def array_to_text_file(path, fidelities):
    with open(path, 'w') as f:
        for item in fidelities:
            f.write(f'{item}\n')
        f.close()

## Runs fidelity tests between original and ZFP
def run_fidelity_tests_zfp():
    for qubits in TEST_CASES['zfp'].keys():
        fidelities = []
        in_file_pure = original_data_path + f'MEM_original_{qubits}/SV_original.out.data'
        for block_size in TEST_CASES['zfp'][qubits]['b']:
            for param in TEST_CASES['zfp'][qubits]['params']:

                if(len(param) == 3):    # Dynamic flag is present
                    in_file_comp = zfp_data_path + f'MEM_zfp_COMP_{qubits}/SV_test_case_{block_size}_{param[0]}_{param[1]}_{param[2]}.out.data'
                else:
                    in_file_comp = zfp_data_path + f'MEM_zfp_COMP_{qubits}/SV_test_case_{block_size}_{param[0]}_{param[1]}.out.data'

                arr_pure = binary_to_array(in_file_pure, pow(2, qubits+1))
                arr_comp = binary_to_array(in_file_comp, pow(2, qubits+1))
                size = len(arr_pure)

                fidelity = calc_fidelity((arr_pure[:size//2], arr_pure[size//2:]), ((arr_comp[:size//2], arr_comp[size//2:])))

                if(len(param) == 3):    # Dynamic flag is present
                    fidelities.append(f'zfp_{block_size}_{param[0]}_{param[1]}_{param[2]} {fidelity}')
                else:
                    fidelities.append(f'zfp_{block_size}_{param[0]}_{param[1]} {fidelity}')
        
        # Done with one qubit
        # Write to file
        array_to_text_file(f'{zfp_out}/zfp_COMP_{qubits}.out', fidelities)


## Runs fidelity tests between original and FPZIP
def run_fidelity_tests_fpzip():
    for qubits in TEST_CASES['fpzip'].keys():
        fidelities = []
        in_file_pure = original_data_path + f'MEM_original_{qubits}/SV_original.out.data'
        for block_size in TEST_CASES['fpzip'][qubits]['b']:
            for param in TEST_CASES['fpzip'][qubits]['params']:

                if(len(param) == 3):    # Dynamic flag is present
                    in_file_comp = fpzip_data_path + f'MEM_fpzip_COMP_{qubits}/SV_test_case_{block_size}_{param[0]}_{param[1]}_{param[2]}.out.data'
                else:
                    in_file_comp = fpzip_data_path + f'MEM_fpzip_COMP_{qubits}/SV_test_case_{block_size}_{param[0]}_{param[1]}.out.data'

                arr_pure = binary_to_array(in_file_pure, pow(2, qubits+1))
                arr_comp = binary_to_array(in_file_comp, pow(2, qubits+1))
                size = len(arr_pure)

                fidelity = calc_fidelity((arr_pure[:size//2], arr_pure[size//2:]), ((arr_comp[:size//2], arr_comp[size//2:])))

                if(len(param) == 3):    # Dynamic flag is present
                    fidelities.append(f'fpzip_{block_size}_{param[0]}_{param[1]}_{param[2]} {fidelity}')
                else:
                    fidelities.append(f'fpzip_{block_size}_{param[0]}_{param[1]} {fidelity}')
        
        # Done with one qubit
        # Write to file
        array_to_text_file(f'{fpzip_out}/fpzip_COMP_{qubits}.out', fidelities)

                
                

def main():
    
    # Setup directory structure
    os.system(f'mkdir fidelity_tests && mkdir {zfp_out} {fpzip_out}')

    #run_fidelity_tests_zfp()
    #run_fidelity_tests_fpzip()

    

    #res = calc_diff_metrics(zfp_data_path + f'MEM_zfp_COMP_14/SV_test_case_512_-a_1e-6.out.data', original_data_path + f'MEM_original_14/SV_original.out.data', pow(2, 13))
    #array_to_text_file('./test.txt', res)

if __name__=="__main__":
    main()