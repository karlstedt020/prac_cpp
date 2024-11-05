import seaborn
import pandas as pd
import os
import sys
import matplotlib.pyplot as plt

df = pd.DataFrame(index=[i for i in range(2, 21, 2)], columns=[i for i in range(200, 2401, 200)], dtype=float)
for x in range(2, 21, 2):
    for y in range(200, 2401, 200):
        avg = 0
        for i in range(5):
            n, m = map(int, input().split())
            res = input()
            time = int(input())
            avg += time
        avg /= 5
        df.loc[m, n] = avg
            
print(df.dtypes)
seaborn.heatmap(df.iloc[::-1], cmap = seaborn.cm.rocket_r)
plt.show()