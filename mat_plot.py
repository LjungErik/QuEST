import matplotlib.pyplot as plt
import re
import math

from numpy import single


def oneLine(qbit):
    y = []
    x = []
    result = open("results.txt", "r")
    for line in result:
        if "ZfpQuestDynamicGrover" in line and f"{qbit} Qubits" in line:
            m = re.search("(?<=is)(.*)", line)
            num = str(m.groups())
            num = num.replace("(' ", "", 1)
            num = num.replace(" sec',)", "", 1)
            x.append(float(num))

    for i in range(len(x)):
        y.append(128 * (2**i))
        y[i] /= 2**7
        y[i] = math.log2(y[i])

    print(y)
    print(x)
    plt.plot(y, x)


oneLine(10)
plt.xlabel("2^(7+n)")
plt.show()
