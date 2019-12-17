#!/bin/bash
#SBATCH -N 32
#SBATCH --ntasks-per-node=8
#SBATCH --time=00:30:00

## Compile the code with mpicc and then run this job with sbatch
srun ./a.out
