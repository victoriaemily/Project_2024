#!/bin/bash

# Array of input sizes (2^16, 2^18, ..., 2^28)
input_sizes=(65536 262144 1048576 4194304 16777216 67108864 268435456)

# Array of MPI process counts
num_procs=(2 4 8 16 32 64 128 256 512 1024)

# Array of input types
input_types=("Sorted" "Random" "ReverseSorted" "1%_perturbed")

# Iterate over all combinations of input_size, processes, and input_type
for input_size in "${input_sizes[@]}"; do
    for proc in "${num_procs[@]}"; do
        for input_type in "${input_types[@]}"; do
            # Submit the job with sbatch, passing the array_size, processes, and input_type as arguments
            sbatch mpi.grace_job $input_size $proc $input_type
        done
    done
done
