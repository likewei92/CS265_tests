import random

N = 1000000
data_min = 0
data_max = 1000000

with open("data.txt", "wb") as fp:
    for _ in range(N):
        fp.write(str(random.randint(data_min,data_max)) + "\n")