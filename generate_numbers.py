import random

N = 10000000
data_min = 0
data_max = 5000

with open("data.txt", "wb") as fp:
    for _ in range(N):
        fp.write(str(random.randint(data_min,data_max)) + "\n")
