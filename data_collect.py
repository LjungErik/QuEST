import os
import time
import resource
import subprocess
from matplotlib.pyplot import get
from numpy import block
import re


def compile_Quest(alg):
    def QuestMasterGrover():
        compileQuest = 'cd ~/QuEST-master/build && rm -rf * && cmake .. -DUSER_SOURCE="grovers_search" && make'
        os.system(compileQuest)

    def QuestMasterBernstein():
        compileQuest = 'cd ~/QuEST-master/build && rm -rf * && cmake .. -DUSER_SOURCE="bernstein_vazirani_circuit" && make'
        os.system(compileQuest)

    def QuestModifiedGrover():
        compileZfpQuest = 'cd ~/QuEST/build && rm -rf * && cmake .. -DUSER_SOURCE="examples/compression_grover_search" && make'
        os.system(compileZfpQuest)

    def QuestModifiedBernstein():
        compileZfpQuest = 'cd ~/QuEST/build && rm -rf * && cmake .. -DUSER_SOURCE="examples/compression_bernstein_vazirani_circuit" && make'
        os.system(compileZfpQuest)

    if alg == "Grover":
        QuestMasterGrover()
        QuestModifiedGrover()
    elif alg == "Bernstein":
        QuestMasterBernstein()
        QuestModifiedBernstein()


# -------------------------------------------------------------------------------------------------

# Execute QuEST and return execution time
def exeQuestTime(argunemt, numOfQbits, blockSize):
    switcher = {
        "Original": ["/home/darko/QuEST-master/build/demo", "-q", f"{numOfQbits}"],
        "ZfpQuest": [
            "/home/darko/QuEST/build/demo",
            "zfp",
            "-q",
            f"{numOfQbits}",
            "-1",
            f"{blockSize}",
            "-r",
            "16",
        ],
        "ZfpQuestDynamic": [
            "/home/darko/QuEST/build/demo",
            "zfp",
            "-q",
            f"{numOfQbits}",
            "-1",
            f"{blockSize}",
            "-r",
            "16",
            "-d",
        ],
        "FpZipQuest": ["/home/darko/QuEST/build/demo", "fpzip", "-q", f"{numOfQbits}"],
    }
    outFile = open("out_file.txt", "a")
    start = time.perf_counter_ns()
    proc = subprocess.Popen(
        switcher.get(argunemt),
        stdout=outFile,
        stderr=subprocess.DEVNULL,
    )
    proc.wait()
    end = time.perf_counter_ns()
    # exeMem = resource.getrusage(resource.RUSAGE_CHILDREN).ru_maxrss
    exeTime = end - start
    outFile.close()
    return exeTime


def writeExeTime(fun, testNum, numOfQbits, blockSize):
    outFile = open("out_file.txt", "a")
    timeResultsFile = open("time_results.txt", "a")
    time_ns = exeQuestTime(fun, numOfQbits, blockSize)
    time_s = time_ns / 10**9
    seg_flag = 0
    n_flag = 0
    for line in open("out_file.txt", "r"):
        # print(line)
        if "Segmentation" in line:
            seg_flag = 1
        if "n_blocks: 1," in line:
            n_flag = 1
    if blockSize >= 32768:
        n_flag = 1
    if seg_flag == 0:
        timeResultsFile.write(
            f"Time test {testNum} Quest {fun} runing {numOfQbits} Qubits and {blockSize} Block Size is {time_s} sec\n"
        )
    outFile.truncate(0)
    outFile.close()
    timeResultsFile.close()
    return n_flag


# """
def GroverTimeExe(numOfQbits, blockSize, groverMaxQubit, numOfTests):
    start_block_size = blockSize
    # Define Flags
    Qbit_flag = 0
    block_flag = 0
    compile_Quest("Grover")
    # Grover Test -----------------------------------------------------------------------------------------------------
    for test in range(numOfTests):
        Qbit_flag = 0
        while Qbit_flag == 0:
            writeExeTime("Original", test, numOfQbits, blockSize)
            writeExeTime("FpZipQuest", test, numOfQbits, blockSize)
            while block_flag == 0:
                block_flag = writeExeTime("ZfpQuest", test, numOfQbits, blockSize)
                block_flag = writeExeTime(
                    "ZfpQuestDynamic", test, numOfQbits, blockSize
                )
                blockSize *= 2

            block_flag = 0
            blockSize = start_block_size

            if numOfQbits >= groverMaxQubit:
                Qbit_flag = 1
            numOfQbits += 1
    # --------------------------------------------------------------------------------------------------------------


def exeQuestMem(argunemt, numOfQbits, blockSize, exitTime):
    switcher = {
        "Original": [
            "/usr/bin/time",
            "-v",
            "timeout",
            f"{exitTime}",
            "/home/darko/QuEST-master/build/demo",
            "-q",
            f"{numOfQbits}",
        ],
        "ZfpQuest": [
            "/usr/bin/time",
            "-v",
            "timeout",
            f"{exitTime}",
            "/home/darko/QuEST/build/demo",
            "zfp",
            "-q",
            f"{numOfQbits}",
            "-1",
            f"{blockSize}",
            "-r",
            "16",
        ],
        "ZfpQuestDynamic": [
            "/usr/bin/time",
            "-v",
            "timeout",
            f"{exitTime}",
            "/home/darko/QuEST/build/demo",
            "zfp",
            "-q",
            f"{numOfQbits}",
            "-1",
            f"{blockSize}",
            "-r",
            "16",
            "-d",
        ],
        "FpZipQuest": [
            "/usr/bin/time",
            "-v",
            "timeout",
            f"{exitTime}",
            "/home/darko/QuEST/build/demo",
            "fpzip",
            "-q",
            f"{numOfQbits}",
        ],
    }
    outFile = open("out_file.txt", "a")
    proc = subprocess.Popen(
        switcher.get(argunemt),
        shell=False,
        stdout=outFile,
        stderr=outFile,
    )
    proc.wait()
    outFile.close()
    # Read output
    outFile = open("out_file.txt", "r")
    for line in outFile:
        if "Maximum resident set size" in line:
            m = re.search("(?<=kbytes)(.*)", line)
            num = str(m.group())
            num = num.replace("): ", "", 1)
    exeMem = int(num)
    return exeMem


def writeExeMem(fun, testNum, numOfQbits, blockSize, exitTime):
    outFile = open("out_file.txt", "a")
    memResultsFile = open("mem_results.txt", "a")
    get_mem = exeQuestMem(fun, numOfQbits, blockSize, exitTime)
    seg_flag = 0
    n_flag = 0
    for line in open("out_file.txt", "r"):
        if "Segmentation" in line:
            seg_flag = 1
        if "n_blocks: 1," in line:
            n_flag = 1
    if blockSize >= 32768:
        n_flag = 1
    if seg_flag == 0:
        memResultsFile.write(
            f"Memory test {testNum} of Quest {fun} runing {numOfQbits} Qubits and {blockSize} Block Size is {get_mem} kb\n"
        )
    outFile.truncate(0)
    outFile.close()
    memResultsFile.close()
    return n_flag


def GroverMemExe(numOfQbits, blockSize, groverMaxQubit, numOfTests, exitTime):
    start_block_size = blockSize
    # Define Flags
    Qbit_flag = 0
    block_flag = 0
    compile_Quest("Grover")
    # Grover Test -----------------------------------------------------------------------------------------------------
    for test in range(numOfTests):
        Qbit_flag = 0
        while Qbit_flag == 0:
            writeExeMem("Original", test, numOfQbits, blockSize, exitTime)
            writeExeMem("FpZipQuest", test, numOfQbits, blockSize, exitTime)
            while block_flag == 0:
                block_flag = writeExeMem(
                    "ZfpQuest", test, numOfQbits, blockSize, exitTime
                )
                block_flag = writeExeMem(
                    "ZfpQuestDynamic", test, numOfQbits, blockSize, exitTime
                )
                blockSize *= 2

            block_flag = 0
            blockSize = start_block_size

            if numOfQbits >= groverMaxQubit:
                Qbit_flag = 1
            numOfQbits += 1


# """

# """
def findMaxQbit(fun, numOfQbits, blockSize):
    # Find Max Qubits --------------------------------------------------------------------------------------------
    compile_Quest("Bernstein")
    maxQbitTime = open("max_Qbit_time.txt", "a")
    done_flag = 0
    block_falg = 0
    while done_flag == 0:
        time_ns = exeQuestTime(fun, numOfQbits, blockSize)
        time_s = time_ns / 10**9
        for line in open("out_file.txt", "r"):
            if "Killed" in line:
                done_flag = 1
                print(f"Maximum num of Qbits for {fun} is {numOfQbits}")
            if "n_blocks: 1," in line:
                block_falg = 1
        maxQbitTime.write(
            f"Success {fun} with {numOfQbits} Qbits, {blockSize} Block Size and time {time_s} sec\n"
        )
        numOfQbits += 1
    maxQbitTime.clsoe()
    outFile = open("out_file.txt", "a")
    outFile.truncate(0)
    outFile.close()
    return block_falg


def vaziraniMaxQbit(startQbit, startBlockSize):
    findMaxQbit("Original", startQbit, startBlockSize)
    findMaxQbit("FpZipQuest", startQbit, startBlockSize)
    blockSize = startBlockSize
    block_flag = 0
    while block_flag == 0:
        block_flag = findMaxQbit("ZfpQuest", startQbit, blockSize)
        block_flag = findMaxQbit("ZfpQuestDynamic", startQbit, blockSize)
        if blockSize >= 32768:
            block_flag = 1


# -------------------------------------------------------------------------------------------------------------------
# """


# GroverTimeExe(numOfQbits=9, blockSize=1024, groverMaxQubit=10, numOfTests=1)
GroverMemExe(
    numOfQbits=25, blockSize=1024, groverMaxQubit=25, numOfTests=1, exitTime=10
)
# vaziraniMaxQbit(startQbit=10, startBlockSize=1024)
