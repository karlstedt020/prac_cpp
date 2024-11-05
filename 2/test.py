import random


n, m, n_proc, t, a, b = map(int, input().split())
print(n, m, n_proc, t)
data = []
for i in range(n):
    data.append(random.randint(a, b))
for j in range(1, 4):
    with open(f"input{j}.txt", "w") as f:
        f.write(f"{n} {m} {n_proc} {j}\n")
        for i in range(n):
            f.write(f"{data[i]} ")
