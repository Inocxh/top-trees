#!/usr/bin/python3

import random
import subprocess
import time
from multiprocessing import Pool

tries                = 10    # Tries for one size
parallel_processes   = 2     # Set max number of processes running at the same time using multiprocessing
test_operations      = 10000 # Operations for one test
size_start           = 100   # Start size of graph
size_step            = 1.25  # Enlarge each step
time_stop_limit      = 3600  # When last step takes longer than X seconds don't start next one

random.seed(0xDEADBEEF)

program = "bin/experiment_edge_weight"
logfile_path  = "experiment_edge_weight.log" # will create .log output file

##################

results = []

def execute(params):
	(size, rnumber) = params
	# Construct and run command
	command = [program, rnumber, str(size), str(test_operations)]
	cmd = subprocess.run(command, stdout=subprocess.PIPE, check=True)

	# Get results
	output = cmd.stdout.decode('utf-8').split()
	result = {
		"size": size,
		"random": rnumber,
		"operations": test_operations,
		"time_top_construction": float(output[0]),
		"time_top_op": float(output[1]),
		"time_topology_construction": float(output[2]),
		"time_topology_op": float(output[3])
	}

	# Log into file and to the stdout
	logline = "{} {} {} {} {} {} {}".format(
		result["random"], result["size"], result["operations"],
		result["time_top_construction"], result["time_top_op"],
		result["time_topology_construction"], result["time_topology_op"]
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
