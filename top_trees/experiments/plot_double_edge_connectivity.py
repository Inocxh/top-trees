#!/usr/bin/python3

import matplotlib.pyplot as plt
import sys

from common import load_values

(sizes, values, yerr) = load_values(
	sys.argv[1], [
		"top_construction", "top_op", "top_op_queries",
		"topology_construction", "topology_op", "topology_op_queries",
		"topology_quick_construction", "topology_quick_op", "topology_quick_op_queries"
	], skip_fields=4
)

################################################################################

# Compute graph
plt.rc('text', usetex=True)
plt.title("Comparison of implementations (time per operation)")

plt.ylabel("Time per 1 operation (sec)")
plt.xlabel("Number of edges")
plt.xscale("log")

plt.errorbar(sizes, values["top_op"], yerr=yerr["top_op"])
plt.errorbar(sizes, values["topology_op"], yerr=yerr["topology_op"])
plt.errorbar(sizes, values["topology_quick_op"], yerr=yerr["topology_quick_op"])

plt.legend(['Self adjusting tress', 'Topology trees', 'Topology trees (expensive updates off)'], loc='upper left')
plt.grid(True)

plt.savefig("double_edge_connectivity_op.pdf")
plt.close()



plt.title("Comparison of implementations (construction time)")

plt.ylabel("Time per 1 edge (sec)")
plt.xlabel("Number of edges")
plt.xscale("log")

plt.errorbar(sizes, values["top_construction"], yerr=yerr["top_construction"])
plt.errorbar(sizes, values["topology_construction"], yerr=yerr["topology_construction"])
#plt.errorbar(sizes, values["topology_quick_construction"], yerr=yerr["topology_quick_construction"])

plt.legend(['Self adjusting tress', 'Topology trees'], loc='upper left')
plt.grid(True)

plt.savefig("double_edge_connectivity_construction.pdf")
plt.close()



plt.title("Comparison of implementations (time per operation, only queries)")

plt.ylabel("Time per 1 operation (sec, log scale)")
plt.xlabel("Number of edges")
plt.xscale("log")
plt.yscale("log")

plt.errorbar(sizes, values["top_op_queries"], yerr=yerr["top_op"])
plt.errorbar(sizes, values["topology_op_queries"], yerr=yerr["topology_op"])
plt.errorbar(sizes, values["topology_quick_op_queries"]) # yerr=yerr["topology_op"]

plt.legend(['Self adjusting tress', 'Topology trees', 'Topology trees (expensive updates off)'], loc='upper left')
plt.grid(True)

plt.savefig("double_edge_connectivity_op_queries.pdf")
plt.close()
