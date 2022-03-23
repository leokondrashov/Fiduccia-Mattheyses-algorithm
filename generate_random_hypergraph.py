#!/bin/python3

import random
import sys

net_num = int(sys.argv[1])
cell_num = int(sys.argv[2])
print(net_num, cell_num)

nets = set([])
while len(nets) < net_num:
    net_size = int((random.random()**6) * (cell_num - 2) + 2) # favor for small sizes
    nets.add(frozenset(random.sample(range(1, cell_num + 1), net_size)))

for net in nets:
    print(' '.join(map(str, net)))

