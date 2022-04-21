from cProfile import label
import sys
import os


import matplotlib.pyplot as plt
import struct
from progress.bar import Bar

## Calculates the difference in the metrics contained in file1 and file2
## @param1  The first file
## @param2  The second file
## @param3  Number of values to be read in respective file
## @return  Tuple containing the maximum difference and average difference
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
        #print("Average val1: " + str(cummulative_val1 / (nr_values/10)) + "\n")
        #print("Average val2: " + str(cummulative_val2 / (nr_values/10)) + "\n")

    plt.plot(vary, val1arr, 'o', label="Quest original",)
    plt.plot(vary, val2arr, '*', label="Quest ZFP",)
    plt.legend()
    plt.show()

    return ("Max diff: ",round(max_diff, 3), "Avg. diff: ", round(tot_diff/nr_values, 5))


## Opens a binary and produces an array of the data inside
## @return  The array read from binary
def binary_to_array(file1, nr_values):

    with Bar("Processing", max=nr_values) as bar:
      
        f1 = open(file1, 'rb')
        
        val1arr = []
        vary = []
        for i in range(nr_values):
            [val1] = struct.unpack('d', f1.read(8))
            val1arr.append(val1)
            vary.append(i)
            bar.next()
        bar.finish()
        f1.close()
    
    return val1arr

## Runs the standard grover algorithm
## @param1  Number of qubits
## @return  void
def run_grover_default(qubits):
    compileQuest = 'cd ../../../../QuEST/build/ && rm -rf * && cmake .. -DPRECISION=2 -DUSER_SOURCE="../examples/grovers_search.c" && make'
    os.system(compileQuest)

    grover_cmd = '../../../../QuEST/build/demo -q {}'.format(qubits) 
    os.system(grover_cmd);

## Runs the grover algorithm using ZFP-compression
## @param1  Number of qubits
## @param2  ZFP dimension
## @param3  ZFP block size
## @param4  ZFP compression rate
## @return  void
def run_grover_zfp(qubits, dims, blocksize, rate):
    compileQuest = 'cd ../build ; rm -rf * && cmake .. -DPRECISION=2  && make'
    os.system(compileQuest)
    
    grover_cmd = '../build/grover zfp -q {} -{} {} -r {} -z '.format(qubits, dims, blocksize, rate) 
    os.system(grover_cmd);

## Dumps array to text-file
## @return void
def array_to_file(array, file):
    with open(file, 'w') as f:
        for val in array:
            f.write("%s\n" % val)

## Runs a test for both the standard QuEST implementation and also QuEST using ZFP-compression using 
## the given parameters.
## @param1  Number of qubits
## @param2  ZFP block size
## @param3  ZFP compression rate
## @return  The average differences between the sections of the two state-vectors
def run_one_test(n_qubits, block_size, rate):
    
    # Clear metrics
    os.system('rm ./grover-search_dump.data ./grover-search_dump_no_compression.data');

    #Remove this later
    os.system('rm ./test_file1.txt ./test_file2.txt');

    # Compile and run quest grover - no compression 
    run_grover_default(n_qubits)

    # Compile and run quest grover 
    run_grover_zfp(n_qubits, 1, block_size, rate)
 
    # Fetch and compare metrics
    metrics = calc_diff_metrics("./grover-search_dump.data", "./grover-search_dump_no_compression.data", 2048)
    

    # Fetch state vector of the standard QuEST implementation
    arr_no_comp = binary_to_array("./grover-search_dump_no_compression.data", pow(2,n_qubits)*2)
    # Fetch state vector of the ZFP QuEST implementation
    arr_zfp = binary_to_array("./grover-search_dump.data", pow(2,n_qubits)*2)

    # Partition both arrays
    sub_arrays_no_comp = sub_divide(arr_no_comp, 32)
    sub_arrays_zfp = sub_divide(arr_zfp, 32)

    # Calculate the average differences between each section of both implementations
    average_diffs = []
    for i in range(len(sub_arrays_zfp)):
        average_diffs.append(calc_avg_diff(sub_arrays_no_comp[i], sub_arrays_zfp[i]))
        print("Average diff no." + str(i) + ":  " + str(average_diffs[i]))

    return  average_diffs
    
    
    
    

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

## Divide an array into sub arrays of size
## @return  Array of sub arrays
def sub_divide(array, size):
    super_array = []
    sub_array = []
    cnt = 0
    

    for i in range(len(array)):
        sub_array.append(array[i])
        cnt += 1
        if(cnt == size):  # Start with next sub-array
            super_array.append(sub_array)
            sub_array = []
            cnt = 0
    
    return super_array
            



## Calculates the average difference between two numerical arrays
## @returns  The average numerical difference between the arrays
def calc_avg_diff(arr1, arr2):

    if(len(arr1) != len(arr2)): # Invalid sizing
        return -1

    if(type(arr1[0]) != int):    # Type sanity check    
        return -1

    cummulative_diff = 0
    length = len(arr1)

    for i in range(length):
        cummulative_diff += abs(arr1[i] - arr2[i])

    return cummulative_diff/length

    
def main():

    # Will run a default instance of QuEST and a ZFP instance of QuEST, compare averages
    # of the state vectors and save to the file "avg_differences.txt"
    # Settings: 20 wubits, 512 block size, 16 compression rate
    array_to_file(run_one_test(20, 512, 16), "avg_differences.txt")
    
    
    
    
    
   
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