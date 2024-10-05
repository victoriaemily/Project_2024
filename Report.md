# CSCE 435 Group project

## 0. Group number: 20

## 1. Group members:
1. Victoria Chen
2. Arielle Shaver
3. Victoria Chiang
4. Bonnie Wu

We are using a group chat over text to communicate.

## 2. Project topic (e.g., parallel sorting algorithms)
Parallel Sorting Algorithms

### 2a. Brief project description (what algorithms will you be comparing and on what architectures)

- Bitonic Sort (Bonnie): 
- Sample Sort (Arielle):
- Merge Sort (Victoria):
- Radix Sort (Vic):

### 2b. Pseudocode for each parallel algorithm
- For MPI programs, include MPI calls you will use to coordinate between processes
- Radix Sort:
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

    1. Get user input for data_type, size (total number of elements), and num_procs (number of processors).

    2. // Initialize MPI
       MPI_Init()  // Set up MPI environment for parallel processing
       task_id = MPI_Comm_rank(MPI_COMM_WORLD)  // Get rank of the current process (task_id)
       num_tasks = MPI_Comm_size(MPI_COMM_WORLD)  // Get the total number of processes available (num_tasks)

    3. // Check if there are enough tasks available
       If num_tasks < 2:
           Print "Need at least two MPI tasks. Quitting..."
           MPI_Abort(MPI_COMM_WORLD, error_code = -1)
           Exit program

    4. Set the number of buckets (m) = num_tasks  // m is the total number of processes, which will sort in parallel.
    
    5. // Synchronize all processes
       MPI_Barrier(MPI_COMM_WORLD)

    6. // Master Process
       If task_id == MASTER:
           Print "Parallel samplesort with master-worker has started with", num_tasks, "tasks."
           Print "Initializing data..."

           // Generate input data of specified data_type
           Generate size amount of input_data of type data_type 

           // Draw a sample of size s
           // Choose s based on some multiple of m (s = m * oversampling_factor)
           s = m * oversampling_factor
           Sample s elements from input_data  // Randomly select s elements for good splitter selection

           // Sort the sampled elements
           Sort the sampled elements using quicksort
           QuickSort(sampled_elements, length(sampled_elements))

           // Select m-1 splitters from the sorted samples
           Select the s/m, 2*(s/m), ..., (m-1)*(s/m) elements as splitters
           // These m-1 splitters will be used to partition the entire dataset into m buckets.

       // Broadcast the splitters to all processes
       MPI_Bcast(splitters, m-1, data_type, root=MASTER, comm=MPI_COMM_WORLD)

    7. // All Processes (Master and Workers)

       // Scatter the input data to all processes
       local_size = size / num_tasks  // Calculate the size of data each process will receive
       local_data = Array[local_size]

       MPI_Scatter(input_data, local_size, data_type, local_data, local_size, data_type, root=MASTER, comm=MPI_COMM_WORLD)
       // Each process now has its portion of the data to work on

       // Each process partitions its data into m buckets
       Local_buckets = Create m empty buckets for partitioning

       // Assign data to buckets based on splitters
       For each element in local_data:
           Determine the correct bucket for the element based on splitters
           Append element to the corresponding bucket in Local_buckets

       // Prepare data for sending to other processes
       // Convert Local_buckets to an appropriate structure for MPI_Alltoallv
       send_counts = [Number of elements in each bucket to send to each process]
       send_displacements = [Offsets for each bucket to be sent]

       // Use MPI_Alltoallv to exchange bucket data among processes
       recv_counts = [Number of elements to receive from each process]  // Allocate space for receiving bucket data
       recv_displacements = [Offsets for each received bucket]

       total_recv_size = sum(recv_counts)  // Total size of received data
       recv_data = Allocate array of size total_recv_size

       MPI_Alltoallv(Local_buckets, send_counts, send_displacements, data_type, recv_data, recv_counts, recv_displacements, data_type, comm=MPI_COMM_WORLD)
       // Each process now has all the elements that belong to its assigned bucket
       local_bucket = recv_data  // This is the bucket each process will sort

       // Sort the received bucket using iterative quicksort
       Print "Task", task_id, "sorting its bucket..."
       QuickSort(local_bucket, length(local_bucket))

    8. // Gather sorted buckets back to the master

       // Use MPI_Gather to gather all sorted buckets
       sorted_bucket_size = length(local_bucket)
       sorted_buckets = None
       If task_id == MASTER:
           sorted_buckets = Array[size]  // Master will gather all sorted data

       MPI_Gather(local_bucket, sorted_bucket_size, data_type, sorted_buckets, sorted_bucket_size, data_type, root=MASTER, comm=MPI_COMM_WORLD)

    9. // Master Process
       If task_id == MASTER:
           // Concatenate all sorted buckets to get the final sorted data
           Final_sorted_data = concatenate(sorted_buckets)
           Print "Parallel samplesort completed."

   10. // Finalize MPI environment
       MPI_Finalize()

End Main Function


// Iterative Quicksort Algorithm Function
QuickSort Function (arr, n):
    // arr: array to be sorted
    // n: size of the array

    Create an empty stack stack
    Push (0, n - 1) onto stack  // Push initial subarray (full range)

    While stack is not empty:
        // Pop high and low indices from stack
        (low, high) = Pop(stack)
        
        // Partition the array
        If low < high:
            // Choose the pivot element as the last element
            pivot = arr[high]
            i = low - 1

            // Rearrange elements based on pivot
            For j from low to high - 1:
                If arr[j] <= pivot:
                    i = i + 1
                    Swap arr[i] and arr[j]

            // Put the pivot in its correct position
            Swap arr[i + 1] and arr[high]
            pivot_index = i + 1

            // Push the left and right subarrays onto the stack
            // Left subarray: elements less than the pivot
            If (pivot_index - 1) > low:
                Push (low, pivot_index - 1) onto stack

            // Right subarray: elements greater than the pivot
            If (pivot_index + 1) < high:
                Push (pivot_index + 1, high) onto stack

End Function

```

### 2c. Evaluation plan - what and how will you measure and compare
- Input sizes: 2^16, 2^18, 2^20, 2^22, 2^24, 2^26, 2^28
- Input types: Sorted, Random, Reverse sorted, 1% perturbed
- Strong scaling (same problem size, increase number of processors/nodes)
- Weak scaling (increase problem size, increase number of processors)
- For number of processors: 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024


For each algorithm (Bitonic Sort, Sample Sort, Merge Sort, Radix Sort), the following metrics will be measured:
- **Execution Time**: Track the time required to complete the sort for various input sizes, input types, and processor counts.
- **Speedup**: Compare the parallel execution time with the sequential baseline to assess the improvement for each algorithm.
- **Efficiency**: Determine how effectively each algorithm uses additional processors by calculating speedup divided by the number of processors.

#### Strong Scaling:
- **Objective**: Analyze how each algorithm performs as the number of processors increases while keeping the input size constant.
- **Method**: For a fixed input size (e.g., 2^24 elements), run the algorithm with 2, 4, 8, 16, 32, and more processors. We will measure execution time and calculate speedup and efficiency for each case.
- **Comparison**: Evaluate how each algorithm’s execution time changes with different processor counts.

#### Weak Scaling:
- **Objective**: Evaluate how well each algorithm handles increasing input sizes as the number of processors grows proportionally, while ensuring each processor has sufficient work to do.

- **Method**: For each input type (Sorted, Random, Reverse Sorted, 1% Perturbed), we will start with a small input size and a reasonable number of processors. As we increase the input size, we plan to proportionally increase the number of processors:
  - 2^16 elements for 2 processors
  - 2^18 elements for 4 processors
  - 2^20 elements for 8 processors
  - 2^22 elements for 16 processors
  - 2^24 elements for 32 processors
  - 2^26 elements for 64 processors
  - 2^28 elements for 128 processors
  - 2^28 elements for 256 processors
  - 2^28 elements for 512 processors
  - 2^28 elements for 1024 processors

- **Comparison**: Measure the execution time for each combination of input size and processor count. The goal is to maintain a stable execution time as both the input size and the number of processors increase, showing strong weak scaling properties.
