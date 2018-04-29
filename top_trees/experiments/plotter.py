#!/usr/bin/python3

import matplotlib.pyplot as plt
import statistics
import sys

sizes = []
values = {"top": [],"topology": []}
yerr = {"top": [],"topology": []}

last_size = 0
run = {"top": [], "topology": []}
with open(sys.argv[1]) as file:
	for line in file:
		parts = line.split()
		size = int(parts[1])
		if last_size != size:
			# Compute previous step
			if len(run["top"]) > 0:
				sizes.append(size)
				values["top"].append(statistics.mean(run["top"]))
				yerr["top"].append(statistics.stdev(run["top"]))
				values["topology"].append(statistics.mean(run["topology"]))
				yerr["topology"].append(statistics.stdev(run["topology"]))
			# Prepare for next step
			last_size = size
			run = {"top": [], "topology": []}
		run["top"].append(float(parts[3]))
		run["topology"].append(float(parts[4]))

# Last step
if len(run["top"]) > 0:
	sizes.append(size)
	values["top"].append(statistics.mean(run["top"]))
	yerr["top"].append(statistics.stdev(run["top"]))
	values["topology"].append(statistics.mean(run["topology"]))
	yerr["topology"].append(statistics.stdev(run["topology"]))


# Compute graph
plt.rc('text', usetex=True)
plt.title("Comparation of implementations")

plt.ylabel("Time per 1 operation (sec)")
plt.xlabel("Number of edges")
plt.xscale("log")

#plt.plot(sizes, values["top"])
plt.errorbar(sizes, values["top"], yerr=yerr["top"])
plt.errorbar(sizes, values["topology"], yerr=yerr["topology"])

plt.legend(['Top', 'Topology'], loc='upper left')
plt.grid(True)

plt.savefig("experiment.pdf")
#plt.show()
