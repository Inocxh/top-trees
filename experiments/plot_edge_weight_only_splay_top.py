#!/usr/bin/python3

import matplotlib.pyplot as plt
import sys

from common import load_values

(sizes1, values1, yerr1) = load_values(sys.argv[1], ["top_construction", "top_op", "topology_construction", "topology_op","splay_construction","splay_op"], skip_fields=3)
(sizes2, values2, yerr2) = load_values(sys.argv[2], ["top_construction", "top_op", "topology_construction", "topology_op","splay_construction","splay_op"], skip_fields=3)
(sizes3, values3, yerr3) = load_values(sys.argv[3], ["top_construction", "top_op", "topology_construction", "topology_op","splay_construction","splay_op"], skip_fields=3)



################################################################################

# Compute graph
plt.rc('text', usetex=True)
plt.title("Comparison of splay top tree operation at different link / cut ratios")

plt.ylabel("Time per 1 operation (sec)")
plt.xlabel("Number of edges")
plt.xscale("log")

plt.errorbar(sizes1, values1["splay_op"], yerr=yerr1["splay_op"])
plt.errorbar(sizes2, values2["splay_op"], yerr=yerr2["splay_op"])
plt.errorbar(sizes3, values3["splay_op"], yerr=yerr3["splay_op"])

plt.legend(['20/80', '50/50', '80/20'], loc='upper left')
plt.grid(True)

plt.savefig("experiment_max_weight_comparison.pdf", bbox_inches='tight')

plt.close()
