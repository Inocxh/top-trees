#!/usr/bin/python3

import random
import subprocess
import time
from multiprocessing import Pool

tries                = 4     # Tries for one size
parallel_processes   = 4     # Set max number of processes running at the same time using multiprocessing
test_operations      = 100   # Operations for one test
size_start           = 10    # Start size of graph (number of vertices)
size_step            = 1.25  # Enlarge each step
time_stop_limit      = 3600  # When last step takes longer than X seconds don't start next one

# get number of edges from number of vertices
def getM(N):
	return N*3;

random.seed(0xDEADBEEF)

program = "bin/experiment_double_edge_connectivity"
logfile_path  = "experiment_double_edge_connectivity.log" # will create .log output file

##################

results = []

def execute(params):
	(N, rnumber) = params
	M = getM(N)
	# Construct and run command
	command = [program, rnumber, str(N), str(M), str(test_operations)]
	cmd = subprocess.run(command, stdout=subprocess.PIPE, check=True)

	# Get results
	output = cmd.stdout.decode('utf-8').split()
	result = {
		"vertices": N,
		"edges": M,
		"random": rnumber,
		"operations": test_operations,
		"time_top_construction": float(output[0]),
		"time_top_op": float(output[1]),
		"time_top_op_queries": float(output[2]),
		"time_topology_construction": float(output[3]),
		"time_topology_op": float(output[4]),
		"time_topology_op_queries": float(output[5]),
		"time_topology_quick_construction": float(output[6]),
		"time_topology_quick_op": float(output[7]),
		"time_topology_quick_op_queries": float(output[8]),
	}

	# Log into file and to the stdout
	logline = "{} {} {} {}   \t{} {} {}   \t{} {} {}   \t{} {} {}".format(
		result["random"], result["vertices"], result["edges"], result["operations"],
		result["time_top_construction"], result["time_top_op"], result["time_top_op_queries"],
		result["time_topology_construction"], result["time_topology_op"], result["time_topology_op_queries"],
		result["time_topology_quick_construction"], result["time_topology_quick_op"], result["time_topology_quick_op_queries"],
	)
	logfile.write(logline+"\n")
	logfile.flush()
	print(logline)
	return result

size = size_start
with open(logfile_path, "w") as logfile:
	while True:
		start_time = time.time()

		# kamenozrout rules
		if size > 1500000:
			parallel_processes = min(5, parallel_processes)
		if size > 3000000:
			parallel_processes = min(2, parallel_processes)
		if size > 6000000:
			parallel_processes = 1

		with Pool(processes=parallel_processes) as pool:
			results.append(pool.map(execute, [(size, '%030x' % random.randrange(16**30)) for i in range(tries)]))

		size = int(size*size_step)
		end_time = time.time()
		print("This iteration: {}s (limit: {}s)".format(end_time-start_time, time_stop_limit))
		if end_time - start_time > time_stop_limit:
			break
