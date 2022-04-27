import matplotlib.pyplot as plt
from progress.bar import Bar
import os
from error_rate_comparison import binary_to_array
import numpy as np
from tqdm import tqdm

def save_plot(pre, post, file):
    x = np.arange(0, len(pre), 1)
    
    plt.ylabel('Values of state vectors')
    plt.xlabel('Index of values')
    plt.plot(x, post, marker = 'o', linestyle = 'none', color='red', label='After')
    plt.plot(x, pre, marker = 'x', linestyle = 'none', color='pink', markersize=2, label='Before')
    plt.legend()
    plt.grid()
    plt.savefig(file)
    plt.clf()

def run_quest_test(qubits):
    grover_cmd = '../build/demo {}'.format(qubits) 
    os.system(grover_cmd)

    # Fetch metrics
    pre_arr = binary_to_array('grover_statevec_dump_pre.data', pow(2,qubits)*2)
    post_arr = binary_to_array('grover_statevec_dump_post.data', pow(2,qubits)*2)

    return (pre_arr, post_arr)

def run_quest_tests():
    for qubits in tqdm(range(10, 17)):
        (pre, post) = run_quest_test(qubits)
        ## Maybe partition here?
        save_plot(pre, post, f'imag_metrics/quest_{qubits}')





def main():
    # Setup dir structure
    os.system(f'mkdir imag_metrics')

    # Run the tests    
    run_quest_tests()

    


if __name__ == "__main__":
    main()