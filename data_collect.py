import os
import time
import re


def compile_Quest(alg):
    def QuestMasterGrover():
        compileQuest = 'cd ~/QuEST-master/build && rm -rf * && cmake .. -DUSER_SOURCE="grovers_search" && make'
        os.system(compileQuest)

    def QuestMasterBernstein():
        compileQuest = 'cd ~/QuEST-master/build && rm -rf * && cmake .. -DUSER_SOURCE="bernstein_vazirani_circuit" && make'
        os.system(compileQuest)

    def QuestModifiedGrover():
        compileZfpQuest = "cd ~/QuEST/build && rm -rf * && cmake .. &&  make"
        os.system(compileZfpQuest)

    if alg == "Grover":
        QuestMasterGrover()
        QuestModifiedGrover()
    elif alg == "Bernstein":
        QuestMasterBernstein()


# -------------------------------------------------------------------------------------------------


# Execute QuEST and return execution time
def exeQuestTime(argunemt: str) -> str:
    switcher = {
        "OriginalGrover": f"~/QuEST-master/build/demo -q {numOfQbuts} >> out_file.txt 2>&1",
        "ZfpQuestGrover": f"~/QuEST/build/grover zfp -q {numOfQbuts} -1 1024 -r 16 >> out_file.txt 2>&1",
        "ZfpQuestDynamicGrover": f"~/QuEST/build/grover zfp -q {numOfQbuts} -1 {blockSize} -r 16 -d >> out_file.txt 2>&1",
    }

    start = time.perf_counter_ns()
    os.system(switcher.get(argunemt))
    end = time.perf_counter_ns()
    exeTime = end - start
    return exeTime


# ---------------------------------------------------------------------------------------------------------------
# Run QuEST with Valgrind for memory check
def exeQuestMemory(argument: str) -> str:
    switcher = {
        "OriginalGrover": f"valgrind --tool=memcheck ~/QuEST-master/build/demo -q {numOfQbuts} >> out_file.txt 2>&1",
        "ZfpQuestGrover": f"valgrind --tool=memcheck ~/QuEST/build/grover zfp -q {numOfQbuts} -1 {blockSize} -r 16 >> out_file.txt 2>&1",
        "ZfpQuestDynamicGrover": f"valgrind --tool=memcheck ~/QuEST/build/grover zfp -q {numOfQbuts} -1 {blockSize} -r 16 -d >> out_file.txt",
    }
    os.system(switcher.get(argument))


# ---------------------------------------------------------------------------------------------------------------------


def writeExeTime(fun):
    outFile = open("out_file.txt", "a")
    timeResultsFile = open("time_results.txt", "a")
    time_ns = exeQuestTime(fun)
    time_s = time_ns / 10**9
    seg_flag = 0
    n_flag = 0
    for line in open("out_file.txt", "r"):
        # print(line)
        if "Segmentation" in line:
            seg_flag = 1
        if "n_blocks: 1," in line:
            n_flag = 1
    if seg_flag == 0:
        timeResultsFile.write(
            f"Time test Quest {fun} runing {numOfQbuts} Qubits and {blockSize} Block Size is {time_s} sec\n"
        )
    if seg_flag == 1:
        timeResultsFile.write(f"{fun} {numOfQbuts} {blockSize} FAILED \n")
    outFile.truncate(0)
    outFile.close()
    timeResultsFile.close()
    return n_flag


def groverTimeResults():
    global numOfQbuts
    global blockSize
    global groverMaxQubit
    global Qbit_flag
    global block_flag
    while Qbit_flag == 0:
        writeExeTime("OriginalGrover")
        while block_flag == 0:
            block_flag = writeExeTime("ZfpQuestGrover")
            block_flag = writeExeTime("ZfpQuestDynamicGrover")
            blockSize *= 2

        block_flag = 0
        blockSize = 128

        if numOfQbuts >= groverMaxQubit:
            Qbit_flag = 1
        numOfQbuts += 1


def writeExeMem(fun):
    outFile = open("out_file.txt", "a")
    memResultsFile = open("mem_results.txt", "a")
    exeQuestMemory(fun)
    seg_flag = 0
    n_flag = 0
    for line in open("out_file.txt", "r"):
        # print(line)
        if "total heap usage:" in line:
            search_tot_mem_used = re.search("(?<=frees, )(.*)", line)
            tot_mem_used = str(search_tot_mem_used.groups())
            tot_mem_used = tot_mem_used.replace("('", "", 1)
            tot_mem_used = tot_mem_used.replace(" bytes allocated',)", "", 1)
            print(tot_mem_used)
        if "Segmentation" in line:
            seg_flag = 1
        if "n_blocks: 1," in line:
            print(numOfQbuts, "Qbuts ", blockSize, "Block Size", "+", line)
            n_flag = 1
    if seg_flag == 0:
        memResultsFile.write(
            f"Memory test Quest {fun} runing {numOfQbuts} Qubits and {blockSize} Block Size is {tot_mem_used} bytes\n"
        )
    if seg_flag == 1:
        memResultsFile.write(f"{fun} {numOfQbuts} {blockSize} FAILED \n")
    # outFile.truncate(0)
    memResultsFile.close()
    outFile.close()
    return n_flag


def groverMemResults():
    global numOfQbuts
    global blockSize
    global groverMaxQubit
    global Qbit_flag
    global block_flag
    while Qbit_flag == 0:
        writeExeTime("OriginalGrover")
        while block_flag == 0:
            block_flag = writeExeTime("ZfpQuestGrover")
            block_flag = writeExeTime("ZfpQuestDynamicGrover")
            blockSize *= 2

        block_flag = 0
        blockSize = 128

        if numOfQbuts >= groverMaxQubit:
            Qbit_flag = 1
        numOfQbuts += 1


# ---------------------------------------------------------------------------------------------------

# compile_Quest("Grover")


# Specify Inputs
numOfQbuts = 5
blockSize = 128
groverMaxQubit = 15
# Define Flags
Qbit_flag = 0
block_flag = 0
# -----------------------------------
groverTimeResults()
groverMemResults()
