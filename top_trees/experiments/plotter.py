#!/usr/bin/python3

import matplotlib.pyplot as plt
import statistics
import sys

variables = ["top_construction", "top_op", "topology_construction", "topology_op"]

values = {}
yerr = {}
run = {}
sizes = []
last_size = 0
for variable in variables:
	values[variable] = []
	yerr[variable] = []
	run[variable] = []


def compute_step():
	if len(run[variables[0]]) == 0:
		return
	sizes.append(last_size)
	for variable in variables:
		values[variable].append(statistics.mean(run[variable]))
		yerr[variable].append(statistics.stdev(run[variable]))
		run[variable] = []  # prepare for next step


with open(sys.argv[1]) as file:
	for line in file:
		parts = line.split()
		size = int(parts[1])
		if last_size != size:
			# Compute previous step
			compute_step()
			# Prepare for next step
			last_size = size
		index = 3
		for variable in variables:
			run[variable].append(float(parts[index]))
			index += 1

# Last step
compute_step()

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
