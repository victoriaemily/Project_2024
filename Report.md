# CSCE 435 Group project

## 0. Group number: 20

## 1. Group members:
1. Victoria Chen
2. Arielle Shaver
3. Victoria Chiang
4. Bonnie Wu

## 2. Project topic (e.g., parallel sorting algorithms)
Parallel Sorting Algorithms

### 2a. Brief project description (what algorithms will you be comparing and on what architectures)

- Bitonic Sort (Bonnie): 
- Sample Sort (Arielle):
- Merge Sort (Victoria):
- Radix Sort (Vic):
```
// Initialize MPI
MPI_Init()
Get the number of processes (n_procs)
Get the rank of each process (rank)

// Main process (rank 0), read and distribute the data
if rank == 0 then
    Read the input array
    Split the input data into n_procs parts
end if

// Scatter the data to all processes given the main process of 0
MPI_Scatter(data, local_data, root=0)

// For each digit LSD to MSD:
for each digit in LSD to MSD do
    // Count the frequency of each digit in the local data
    local_histogram = calculate_histogram(local_data, digit)
    
    // Share and combine all local histograms to create a global histogram
    global_histogram = MPI_Allreduce(local_histogram, MPI_SUM)

    // Use the global histogram to figure out where data needs to go
    prefix_sum = calculate_prefix_sum(global_histogram)
    
    // Exchange data between processes based on prefix_sum
    MPI_Alltoallv(local_data, send_counts, recv_counts, sorted_data)

    // Update local data with the received sorted data
    local_data = sorted_data
end for

// Gather all the sorted data back to process 0
MPI_Gather(local_data, sorted_array, root=0)

// Finalize MPI (clean up and end the program)
MPI_Finalize()

```

### 2b. Pseudocode for each parallel algorithm
- For MPI programs, include MPI calls you will use to coordinate between processes

- Merge Sort:
```
    Main:  
        1. Initialize MPI - MPI_Init()
            a. MPI_INIT(&argc, &argv)
        2. Rank and size of MPI COMM_WORLD
            a. MPI_Comm_rank, MPI_Comm_size
        3. If rank == 0 then: read in the input array, split the input array into the number of processes part
            a. Use a fixed array for sorting so it is the same across all different types of sorting 
            b. no variable is our size of array 
        4. Broadcast that size of array to all the processes using MPI_Bcast
            a. MPI_Bcast(no, 1, MPI_INT, 0, MPI_COMM_WORLD)
        5. Find the size of each subarray for each of the process: subSize = no  / world size (from mpi_comm_size)
        6. Allocate a memory for each process’s subarray with subSize
        7. Scatter the original array into all the subarrays
            a. MPI_Scatter()
        8. Allocate temp array for merging with subSize
        9. Every process will perform merge sort on its own subarray 
            a. mergeSort(subarray, temparray, 0, subSize-1)
        10. Root process - gathers all the sorted subarrays 
            a. World rank is 0, we want to make a sorted array with the size of the array (no)
        11. Perform MPI_Gather to grab all the sorted subarrays from all the processes 
        12. Final merge with the root process so world rank is 0, create another array with the no variable size and use mergeSort function
        13. Add a print statement that prints out the sorted array to check 
        14. Free any memory and finalize the MPI environment: MPI_Finalize()


    Merge Function: * merge two sorted halves of one array into a complete sorted single array*
        1. Set lh= to the start index to the left half, set i= to the start index for the temporary array (temp array), 
            set rh= to the starting index of the right half (ending index of the first half + 1)
        2. While loop to start merging and compare the elements in both halves: * loop will finish after one half is processed* -> 
            While (lh <= ending index of first half && rh <= ending index of that array portion needed to sort)
                a. If current element array[lh] is <= to the current element in the right half so array[rh] we want to copy the array[lh] into the temp array at i and then increment the lh and the i
                b. Else we want to do the opposite so array[rh] to temp array at i and increment the rh and the i
        3. Remaining elements will be copied:
            a. If there are elements in the right half leftover or the left it will be copied into the temp array 
            b. Check the index and the end of side index to see if one side has been fully processed or not 
        4. Copy the sorted elements from the temp array all back to the original array
            a. using a for loop and the indexes 

    MergeSort Function: *recursive merge sort algorithm without impacting any MPI*
        1. Check if the starting index is less than the ending index of the array -> if LI < RI -> want to make sure the array 
            has more than one element
                a. Find the middle index: mid = (LI + RI) / 2, average of left and right index divided by 2
                b. Recursively call mergeSort so it can sort the left half -> LI to mid, left index to mid index 
                c. Recursively call mergeSort so it can sort the right half -> mid+1, RI, mid+1 index to right index
                d. Merge Function called so we can combine it into a single sorted array 
	
```
- Sample Sort:
```
Main Function:

    1. Get user input for data_type, size, num_procs, and oversampling_factor (k)

    2. // Initialize MPI
        Initialize MPI  // Set up MPI environment for parallel processing
        Get rank of the current process (task_id)  // Determine if the current process is the master or a worker
        Get the total number of processes (num_tasks)  // Get the total number of processes available

    3. // Check if there are enough tasks available
         If num_tasks < 2:
            Print "Need at least two MPI tasks. Quitting..."  // Ensure at least one master and one worker are available
            Abort MPI with error code (use MPI_Abort)  // Abort the program if there are insufficient processes
            Exit program  // Exit as parallel sorting cannot proceed with fewer than 2 processes

    4. Set the number of buckets (p) = num_tasks - 1
       // The number of buckets is set to the number of worker processes (master does not count as a worker)

    5.  // Synchronize all processes to start computation using MPI Barrier 
        MPI_Barrier()  // Ensure all processes reach this point before proceeding, so everyone starts at the same time

    6. // Master Process
        If task_id == MASTER:
            Print "Parallel samplesort with master-worker has started with num_tasks tasks."
            Print "Initializing data..."

            // Generate input data of specified data_type
            Generate 'size' amount of input_data of type 'data_type'  
            // Create the initial data to be sorted, e.g., an array of integers or floats

            // Sample n * k elements from input_data
            Sample 'p * oversampling_factor' elements from input_data  
            // Randomly select elements for sampling to ensure good splitters
            Sort the sampled elements  
            // Sort the sampled elements to help determine appropriate bucket boundaries
            Select the k-th, 2*k-th, ..., (p-1)*k-th elements as splitters  
            // Use these sampled elements to determine splitters for partitioning the data into buckets

            // Partition the data into buckets based on splitters
            Create p buckets based on the splitters  
            // Each bucket will hold data that falls within a specific range defined by the splitters
            Loop over input_data:
                Place each element in the appropriate bucket  
                // For each element, determine which bucket it belongs to based on the splitters

            // Send buckets to worker tasks
            For each worker process from 1 to numworkers:
                Send the assigned bucket to each worker using MPI_Send  
                // Use MPI to send each bucket to the corresponding worker for sorting
                Print "Sending bucket to task", worker_process_id  
                // Log the process of sending buckets to keep track of distribution

            // Master sorts its own bucket
            Print "Master sorting its own bucket..."
            masterBucket = bucket[MASTER]  
            // The master will sort its own portion of data
            low = 0
            high = length(masterBucket) - 1

            // Sorting the bucket
            Create a stack to keep track of subarrays  
            // Use a stack for iterative quicksort (to avoid recursive function calls)
            Push (low, high) onto the stack  
            // Start sorting the entire range of the master's bucket

            While stack is not empty:
                Pop (low, high) from the stack  
                // Get the current subarray to sort

                If low < high:
                    pivot = masterBucket[high]  
                    // Select the last element as the pivot for partitioning
                    i = low - 1  
                    // Initialize the index of the smaller element

                    // Partitioning the array
                    For j from low to high - 1:
                        If masterBucket[j] <= pivot:
                            i = i + 1
                            Swap masterBucket[i] and masterBucket[j]  
                            // Swap elements to ensure all elements less than the pivot are on the left

                    Swap masterBucket[i + 1] and masterBucket[high]  
                    // Place the pivot element in its correct sorted position
                    pivot_index = i + 1

                    // Push the subarrays onto the stack to sort them later
                    Push (low, pivot_index - 1) onto the stack  
                    // Push the left subarray (elements less than the pivot) onto the stack
                    Push (pivot_index + 1, high) onto the stack  
                    // Push the right subarray (elements greater than the pivot) onto the stack

            // Gather sorted buckets from worker tasks
            Set message type (mtype) = FROM_WORKER

            For each worker process from 1 to numworkers:
                Receive sorted bucket from worker using MPI_Receive  
                // Receive the sorted bucket from each worker
                Append sorted bucket to final sortedData  
                // Collect all sorted buckets to form the final sorted result
                Print "Received sorted bucket from task", worker_process_id  
                // Log the receipt of sorted data from each worker

    7. // Worker Processes
        Else If task_id > MASTER:
            // Receive bucket data from master
            Set message type (mtype) = FROM_MASTER
            Receive the assigned bucket from master using MPI_Receive  
            // Receive the data bucket that the master has assigned to this worker
            worker_bucket = received_bucket

            // Sort the received_bucket
            Print "Worker", task_id, "sorting its assigned bucket..."
            low = 0
            high = length(worker_bucket) - 1

            Create a stack to keep track of subarrays  
            // Use a stack to perform iterative quicksort on the received bucket
            Push (low, high) onto the stack  
            // Start sorting the entire range of the worker's bucket

            While stack is not empty:
                Pop (low, high) from the stack  
                // Get the current subarray to sort

                If low < high:
                    pivot = worker_bucket[high]  
                    // Choose the last element as the pivot for partitioning
                    i = low - 1

                    // Partitioning the array
                    For j from low to high - 1:
                        If worker_bucket[j] <= pivot:
                            i = i + 1
                            Swap worker_bucket[i] and worker_bucket[j]  
                            // Swap elements to place them correctly relative to the pivot

                    Swap worker_bucket[i + 1] and worker_bucket[high]  
                    // Place the pivot in its correct sorted position
                    pivot_index = i + 1

                    // Push the subarrays onto the stack to sort them later
                    Push (low, pivot_index - 1) onto the stack  
                    // Push the left subarray onto the stack for sorting
                    Push (pivot_index + 1, high) onto the stack  
                    // Push the right subarray onto the stack for sorting

            // Send sorted bucket back to master
            Print "Worker", task_id, "sending sorted bucket back to master..."
            Set message type (mtype) = FROM_WORKER
            Send sorted worker_bucket to master using MPI_Send  
            // Send the sorted data back to the master for final merging

    8. // Finalize MPI environment
        Finalize MPI  

End Main Function

```

### 2c. Evaluation plan - what and how will you measure and compare
- Input sizes: 2^16, 2^18, 2^20, 2^22, 2^24, 2^26, 2^28
- Input types: Sorted, Random, Reverse sorted, 1% perturbed
- Strong scaling (same problem size, increase number of processors/nodes)
- Weak scaling (increase problem size, increase number of processors)
- For number of processors: 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024
