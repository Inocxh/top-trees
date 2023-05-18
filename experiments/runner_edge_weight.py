#!/usr/bin/python3

import sys
import random
import subprocess
import time
from statistics import median
from multiprocessing import Pool


tests                = 8    # Tries for one size
measurements		 = 3	# measurements per seed
warmup 				 = 5 	# number of warmup rounds before a measurement is taken
workload 			 = float(sys.argv[1]) # Ratio of insert / destroy operations
test_operations      = 1000   # Operations for one test
size_start           = 450    # Start size of graph
size_step            = 1.25  # Enlarge each step
time_stop_limit      = 3600  # When last step takes longer than X seconds don't start next one

random.seed(0xDEADBEEF)

program = "bin/experiment_edge_weight"
logfile_path  = f"experiment_edge_weight_{workload}.log" # will create .log output file

##################

results = []

def execute(params):
	(size, rnumber) = params

	measurement_results = [None for i in range(measurements)]
	for i in range(measurements):
		# Construct and run command
		command = [program, rnumber, str(size), str(test_operations), str(warmup), str(workload)]
		cmd = subprocess.run(command, stdout=subprocess.PIPE, check=True)

		# Get results
		output = cmd.stdout.decode('utf-8').split()
		measurement_results[i] = {
			"size": size,
			"random": rnumber,
			"operations": test_operations,
			"time_top_construction": float(output[0]),
			"time_top_op": float(output[1]),
			"time_topology_construction": float(output[2]),
			"time_topology_op": float(output[3]),
			"time_splay_construction": float(output[4]),
			"time_splay_op": float(output[5])
		}
	#print(command)

	result = {}; 
	result["size"] = size
	result["random"] = rnumber
	result["operations"] = test_operations
	result["time_top_construction"] = median([measurement_results[i]["time_top_construction"] for i in range(measurements)])
	result["time_top_op"] = median([measurement_results[i]["time_top_op"] for i in range(measurements)])
	result["time_topology_construction"] = median([measurement_results[i]["time_topology_construction"] for i in range(measurements)])
	result["time_topology_op"] = median([measurement_results[i]["time_topology_op"] for i in range(measurements)])
	result["time_splay_construction"] = median([measurement_results[i]["time_splay_construction"] for i in range(measurements)])
	result["time_splay_op"] = median([measurement_results[i]["time_splay_op"] for i in range(measurements)])


	# Log into file and to the stdout
	logline = "{} {} {} {} {} {} {} {} {}".format(
		result["random"], result["size"], result["operations"],
		result["time_top_construction"], result["time_top_op"],
		result["time_topology_construction"], result["time_topology_op"],
		result["time_splay_construction"], result["time_splay_op"]
	)
	#if logging:
		# logfile.write(logline+"\n")
		# logfile.flush()
		
	print(logline)
	return result

logging = True
size = size_start
#with open(logfile_path, "w") as logfile:
while True:
	start_time = time.time()



	for i in range(tests):
		execute((size, '%030x' % random.randrange(16**30),))

	size = int(size*size_step)
	end_time = time.time()
	print("This iteration: {}s (limit: {}s)".format(end_time-start_time, time_stop_limit))
	if end_time - start_time > time_stop_limit:
		break
