data = [1,26829382,33,
2,26829183,29,
3,26829093,47,
4,26828666,54,
5,26828946,50,
6,26828906,48,
7,26828507,77,
8,26828386,87,
9,26828336,85,
10,26828734,84,
11,26828902,52,
12,26828574,69,
13,26828602,94,
14,26828349,90]
import numpy as np

data = np.array(data)
data = data.reshape((14, 3))
import matplotlib.pyplot as plt

fig, ax = plt.subplots()
ax.plot(data[:, 0], data[:, 2])
ax.set_xlabel("Number of processes")
ax.set_ylabel("Time, seconds")
plt.show()