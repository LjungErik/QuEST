from cProfile import label
import sys
import os

import matplotlib.pyplot as plt
import struct
from progress.bar import Bar


def usage():
    print("usage: main.py <original_file> <decompressed_file> <number_of_vaules>")

def calc_diff_metrics(file1, file2, nr_values):

    tot_diff = 0.0
    max_diff = 0.0

    with Bar("Processing", max=nr_values) as bar:
        f1 = open(file1, 'rb')
        f2 = open(file2, 'rb')
        val1arr = []
        val2arr = []
        vary = []
        for i in range(nr_values):
            [val1] = struct.unpack('f', f1.read(4))
            [val2] = struct.unpack('f', f2.read(4))
            val1arr.append(val1)
            val2arr.append(val2)
            vary.append(i)
            diff = abs(val2 - val1)
            tot_diff += diff
            if max_diff < diff:
                max_diff = diff
            bar.next()
        bar.finish()
        f1.close()
        f2.close()
        return ("Max diff: ",round(max_diff, 3), "Avg. diff: ", round(tot_diff/nr_values, 3))

def run_grover_default(qubits):
    compileQuest = 'cd ../../../../QuEST/build/ && rm -rf * && cmake .. -DPRECISION=2 -DUSER_SOURCE="../examples/grovers_search.c" && make'
    os.system(compileQuest)

    grover_cmd = '../../../../QuEST/build/demo {}'.format(qubits) 
    os.system(grover_cmd);


def run_grover_zfp(qubits, dims, blocksize, rate):
    compileQuest = 'cd ../build ; rm -rf * && cmake .. -DPRECISION=2  && make'
    os.system(compileQuest)
    
    grover_cmd = '../build/grover zfp -q {} -{} {} -r {}'.format(qubits, dims, blocksize, rate) 
    os.system(grover_cmd);

def main():

    n_qubits = 10
    # Clear metrics
    os.system('rm ./grover-search_dump.data ./grover-search_dump_no_compression.data');

    #Remove this later
    os.system('rm ./test_file1.txt ./test_file2.txt');

    # Compile and run quest grover - no compression 
    run_grover_default(n_qubits)

    # Compile and run quest grover - zfp compression(qubits=10, dim=1, blocksize=256, rate=1)
    run_grover_zfp(n_qubits, 1, 512, 16)
    

    # Fetch and compare metrics
    metrics = calc_diff_metrics("./grover-search_dump.data", "./grover-search_dump_no_compression.data", 1024)
    print(metrics)

""" 
    if len(sys.argv) < 4:
        usage()
        exit(-1)

    file1 = sys.argv[1]
    file2 = sys.argv[2]
    nr_values = int(sys.argv[3])
    
 """
    


    #print(
        #f"Max Diff: {round(max_diff, 3)}, Avg Diff: {round(tot_diff/nr_values, 3)}")
    #plt.plot(vary, val1arr, 'o', label="Quest original",)
    #plt.plot(vary, val2arr, '*', label="Quest ZFP",)
    #plt.legend()
    #plt.show()


if __name__ == "__main__":
    main()