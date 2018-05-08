import statistics

def load_values(filename, variables, skip_fields=0):
	values = {}
	yerr = {}
	sizes = []

	run = {}
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

	with open(filename) as file:
		for line in file:
			parts = line.split()
			size = int(parts[1])
			if last_size != size:
				# Compute previous step
				compute_step()
				# Prepare for next step
				last_size = size
			index = skip_fields
			for variable in variables:
				run[variable].append(float(parts[index]))
				index += 1

	# Last step
	compute_step()

	return sizes, values, yerr
