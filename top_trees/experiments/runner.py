#!/usr/bin/python3

import random
import subprocess
import time

tries           = 5     # Tries for one size
test_operations = 1000  # Operations for one test
size_start      = 100   # Start size of graph
size_step       = 1.25  # Enlarge each step
time_stop_limit = 10    # When last step takes longer than X seconds don't start next one

random.seed(0xDEADBEEF)

program = "bin/experiment_edge_weight"
logfile_path  = "experiment_edge_weight.log" # will create .log output file

##################

results = []

size = size_start
with open(logfile_path, "w") as logfile:
	while True:
		start_time = time.time()
		for i in range(tries):
			# Random HEX number
			rnumber = '%030x' % random.randrange(16**30)

			# Construct and run command
			command = [program, rnumber, str(size), str(test_operations)]
			cmd = subprocess.run(command, stdout=subprocess.PIPE, check=True)

			# Get results
			output = cmd.stdout.decode('utf-8').split()
			result = {
				"size": size,
				"random": rnumber,
				"operations": test_operations,
				"time_top": float(output[0]),
				"time_top_op": float(output[1]),
				"time_topology": float(output[2]),
				"time_topology_op": float(output[3])
			}

			# Log into file and to the stdout
			logline = "{} {} {} {} {}".format(result["random"], result["size"], result["operations"], result["time_top_op"], result["time_topology_op"])
			logfile.write(logline+"\n")
			logfile.flush()
			print(logline)
			results.append(result)

		size = int(size*size_step)
		end_time = time.time()
		if end_time - start_time > time_stop_limit:
			break
