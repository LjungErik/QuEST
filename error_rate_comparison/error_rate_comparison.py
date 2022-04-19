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
        cummulative_val1 = 0
        cummulative_val2 = 0
        f1 = open(file1, 'rb')
        f2 = open(file2, 'rb')
        val1arr = []
        val2arr = []
        vary = []
        for i in range(nr_values):
            [val1] = struct.unpack('d', f1.read(8))
            [val2] = struct.unpack('d', f2.read(8))
            if(i % 10 == 0):
                #print("Val1: " + str(val1) + "       Val2: " + str(val2) +"\n")
                cummulative_val1 += val1;
                cummulative_val2 += val2;
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
        print("Average val1: " + str(cummulative_val1 / (nr_values/10)) + "\n")
        print("Average val2: " + str(cummulative_val2 / (nr_values/10)) + "\n")

    plt.plot(vary, val1arr, 'o', label="Quest original",)
    plt.plot(vary, val2arr, '*', label="Quest ZFP",)
    plt.legend()
    plt.show()

    return ("Max diff: ",round(max_diff, 3), "Avg. diff: ", round(tot_diff/nr_values, 5))

def run_grover_default(qubits):
    compileQuest = 'cd ../../../../QuEST/build/ && rm -rf * && cmake .. -DPRECISION=2 -DUSER_SOURCE="../examples/grovers_search.c" && make'
    os.system(compileQuest)

    grover_cmd = '../../../../QuEST/build/demo -q {}'.format(qubits) 
    os.system(grover_cmd);


def run_grover_zfp(qubits, dims, blocksize, rate):
    compileQuest = 'cd ../build ; rm -rf * && cmake .. -DPRECISION=2  && make'
    os.system(compileQuest)
    
    grover_cmd = '../build/grover zfp -q {} -{} {} -r {}'.format(qubits, dims, blocksize, rate) 
    os.system(grover_cmd);

def run_one_test(n_qubits):

    # Clear metrics
    os.system('rm ./grover-search_dump.data ./grover-search_dump_no_compression.data');

    #Remove this later
    os.system('rm ./test_file1.txt ./test_file2.txt');

    # Compile and run quest grover - no compression 
    run_grover_default(n_qubits)

    # Compile and run quest grover - zfp compression(qubits=10, dim=1, blocksize=256, rate=16)
    run_grover_zfp(n_qubits, 1, 512, 16)
    

    # Fetch and compare metrics
    metrics = calc_diff_metrics("./grover-search_dump.data", "./grover-search_dump_no_compression.data", 2048)
    print(metrics)

    return metrics

""" 
    if len(sys.argv) < 4:
        usage()
        exit(-1)

    file1 = sys.argv[1]
    file2 = sys.argv[2]
    nr_values = int(sys.argv[3])
    
    print(
        f"Max Diff: {round(max_diff, 3)}, Avg Diff: {round(tot_diff/nr_values, 3)}")
    plt.plot(vary, val1arr, 'o', label="Quest original",)
    plt.plot(vary, val2arr, '*', label="Quest ZFP",)
    plt.legend()
    plt.show()
    
 """
    
def main():

    run_one_test(10)
    max_diff_list = []
    avg_diff_list = []


    

    """
    for qubits in range(10, 12):
        result = run_one_test(qubits)
        max_diff_list.append((qubits, result[1])) # Append the max diff value
        avg_diff_list.append((qubits, result[3])) # Append the average diff value
    
    print("Max diff list is:\n")
    print(max_diff_list)
    print("\n")
    print("Max avg list is:\n")
    print(avg_diff_list)

    """
    


if __name__ == "__main__":
    main()