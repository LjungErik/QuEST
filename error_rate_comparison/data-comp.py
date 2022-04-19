from cProfile import label
import sys

import matplotlib.pyplot as plt
import struct
from progress.bar import Bar


def usage():
    print("usage: main.py <original_file> <decompressed_file> <number_of_vaules>")


def main():

    if len(sys.argv) < 4:
        usage()
        exit(-1)

    file1 = sys.argv[1]
    file2 = sys.argv[2]
    nr_values = int(sys.argv[3])

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

    print(
        f"Max Diff: {round(max_diff, 3)}, Avg Diff: {round(tot_diff/nr_values, 3)}")
    plt.plot(vary, val1arr, 'o', label="Quest original",)
    plt.plot(vary, val2arr, '*', label="Quest ZFP",)
    plt.legend()
    plt.show()


if __name__ == "__main__":
    main()