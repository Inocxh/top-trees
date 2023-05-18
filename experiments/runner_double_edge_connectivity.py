#!/usr/bin/python3

import sys
import random
import subprocess
import time
from statistics import median
from multiprocessing import Pool


tests 				 = 4 	 
measurements 		 = 3
warmup 				 = 1
workload 			 = float(sys.argv[1])
only_splay 			 = int(sys.argv[2]);
test_operations      = 500   # Operations for one test
size_start           = 100   # Start size of graph (number of vertices)
size_step            = 1.25  # Enlarge each step
time_stop_limit      = 3600  # When last step takes longer than X seconds don't start next one

# get number of edges from number of vertices
def getM(N):
	return N*3

random.seed(0xDEADBEEF)

program = "bin/experiment_double_edge_connectivity"
#logfile_path  = f"experiment_double_edge_connectivity_{workload}.log" # will create .log output file

##################

results = []

def execute(params):
	(N, rnumber) = params
	M = getM(N)

	measurement_results = [None for i in range(measurements)]
	for i in range(measurements): 

		# Construct and run command
		command = [program, rnumber, str(N), str(M), str(test_operations), str(warmup), str(workload), str(only_splay)]
		#print(command)
		cmd = subprocess.run(command, stdout=subprocess.PIPE, check=True)

		# Get results
		output = cmd.stdout.decode('utf-8').split()
		measurement_results[i] = {
			"vertices": N,
			"edges": M,
			"random": rnumber,
			"operations": test_operations,
			"time_top_construction": float(output[0]),
			"time_top_op": float(output[1]),
			"time_topology_construction": float(output[2]),
			"time_topology_op": float(output[3]),
			"time_splay_construction": float(output[4]),
			"time_splay_op": float(output[5]),
		}
	result = {
		"vertices": N,
		"edges": M,
		"random": rnumber,
		"operations": test_operations,
		"time_top_construction"				: median([measurement_results[i]["time_top_construction"] 		for i in range(measurements)]),
		"time_top_op"						: median([measurement_results[i]["time_top_op"] 				for i in range(measurements)]),
		"time_topology_construction"		: median([measurement_results[i]["time_topology_construction"] 	for i in range(measurements)]),
		"time_topology_op"					: median([measurement_results[i]["time_topology_op"] 			for i in range(measurements)]),
		"time_splay_construction"			: median([measurement_results[i]["time_splay_construction"] 	for i in range(measurements)]),
		"time_splay_op"						: median([measurement_results[i]["time_splay_op"] 				for i in range(measurements)]),
	}


	# Log into file and to the stdout
	logline = "{} {} {} {}   \t{} {}   \t{} {}   \t {} {}".format(
		result["random"], result["vertices"], result["edges"], result["operations"],
		result["time_top_construction"], result["time_top_op"],
		result["time_topology_construction"], result["time_topology_op"],
		result["time_splay_construction"], result["time_splay_op"],
	)
	#logfile.write(logline+"\n")
	#logfile.flush()
	print(logline)
	return result

size = size_start
#with open(logfile_path, "w") as logfile:
while True:
	start_time = time.time()

	for i in range(tests):
		execute((size, '%030x' % random.randrange(16**30)))

	size = int(size*size_step)
	end_time = time.time()
	#print("This iteration: {}s (limit: {}s)".format(end_time-start_time, time_stop_limit))
	if end_time - start_time > time_stop_limit:
		break
