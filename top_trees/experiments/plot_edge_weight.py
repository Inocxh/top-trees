#!/usr/bin/python3

import matplotlib.pyplot as plt
import sys

from common import load_values

(sizes, values, yerr) = load_values(sys.argv[1], ["top_construction", "top_op", "topology_construction", "topology_op"], skip_fields=3)

################################################################################

# Compute graph
plt.rc('text', usetex=True)
plt.title("Comparison of implementations (time per operation)")

plt.ylabel("Time per 1 operation (sec)")
plt.xlabel("Number of edges")
plt.xscale("log")

plt.errorbar(sizes, values["top_op"], yerr=yerr["top_op"])
plt.errorbar(sizes, values["topology_op"], yerr=yerr["topology_op"])

plt.legend(['Self adjusting tress', 'Topology trees'], loc='upper left')
plt.grid(True)

plt.savefig("experiment_op.pdf")
#plt.show()
plt.close()


plt.title("Comparison of implementations (construction time)")

plt.ylabel("Time per 1 edge (sec)")
plt.xlabel("Number of edges")
plt.xscale("log")

plt.errorbar(sizes, values["top_construction"], yerr=yerr["top_construction"])
plt.errorbar(sizes, values["topology_construction"], yerr=yerr["topology_construction"])

plt.legend(['Self adjusting tress', 'Topology trees'], loc='upper left')
plt.grid(True)

plt.savefig("experiment_construction.pdf")
