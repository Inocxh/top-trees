#!/bin/sh 

### ---------------
### General options 
### ---------------


### -- specify queue -- 
#BSUB -q hpc

### -- set the job Name -- 
#BSUB -J TopTreeBench_connec_splay_08

### -- ask for number of cores (default: 1) -- 
#BSUB -n 1 

### -- specify that we need 4GB of memory per core/slot -- 
#BSUB -R "rusage[mem=16GB]"
#BSUB -R "select[model == XeonE5_2660v3]"

### -- set walltime limit: hh:mm -- 
#BSUB -W 24:00 

### -- set the email address -- 
#BSUB -u s204472@student.dtu.dk

### -- send notification at completion -- 
#BSUB -N 

### -- Specify the output and error file. %J is the job-id -- 
### -- -o and -e mean append, -oo and -eo mean overwrite -- 
#BSUB -o logs/Output_%J.out 
#BSUB -e logs/Output_%J.err 

# here follow the commands you want to execute with input.in as the input file

python3 experiments/runner_double_edge_connectivity.py 0.8 0 > out/connec_splay_08.out




