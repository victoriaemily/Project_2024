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
Bitonic Sort
```
MAIN BEGIN
    // Initialize MPI
    MPI_Init()
    
    // Get the number of processes
    MPI_Comm_size()
    
    // Get the rank/ID of the process
    MPI_Comm_rank()
    
    // Scatter data among processes
    MPI_Scatter()
    
    // Bitonic Sort process
    for (i = 0; i < log(number processes); i++) {  // log(n) stages of sorting, also creates distance between partners as sorting progresses
        for (j = i; j >= 0; j--) {      // Comparing pairs (ex: if i = 2, j takes the values 2, 1, and 0)
            if ((process_rank >> (i + 1)) % 2 == 0 && (process_rank >> j) % 2 == 0) || ((process_rank >> (i + 1)) % 2 != 0 && (process_rank >> j) % 2 != 0){ 
		// Use rightshift operator to determine which processes are swapping for the current stage of sorting (ascending or descending order)
                // Both ranks are even or both are odd
                Swap elements in ascending order (smaller rank will have smaller number)
                (send maximum value to partner
                receive minimum value from partner)
            } 
            else if ((process_rank >> (i + 1)) % 2 != 0 && (process_rank >> j) % 2 != 0) {
                // One rank is even and one is odd
                Swap elements in descending order (vice versa to the ascending one)
                (send minimum value to partner
                receive maximum value from partner)
            }
        }
    }

    // Gather results back to the root process
    MPI_Gather()
    
    // Finalize MPI
    MPI_Finalize()
MAIN END
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


### 3a. Caliper instrumentation
Please use the caliper build `/scratch/group/csce435-f24/Caliper/caliper/share/cmake/caliper` 
(same as lab2 build.sh) to collect caliper files for each experiment you run.

Your Caliper annotations should result in the following calltree
(use `Thicket.tree()` to see the calltree):
```
main
|_ data_init_X      # X = runtime OR io
|_ comm
|    |_ comm_small
|    |_ comm_large
|_ comp
|    |_ comp_small
|    |_ comp_large
|_ correctness_check
```

Required region annotations:
- `main` - top-level main function.
    - `data_init_X` - the function where input data is generated or read in from file. Use *data_init_runtime* if you are generating the data during the program, and *data_init_io* if you are reading the data from a file.
    - `correctness_check` - function for checking the correctness of the algorithm output (e.g., checking if the resulting data is sorted).
    - `comm` - All communication-related functions in your algorithm should be nested under the `comm` region.
      - Inside the `comm` region, you should create regions to indicate how much data you are communicating (i.e., `comm_small` if you are sending or broadcasting a few values, `comm_large` if you are sending all of your local values).
      - Notice that auxillary functions like MPI_init are not under here.
    - `comp` - All computation functions within your algorithm should be nested under the `comp` region.
      - Inside the `comp` region, you should create regions to indicate how much data you are computing on (i.e., `comp_small` if you are sorting a few values like the splitters, `comp_large` if you are sorting values in the array).
      - Notice that auxillary functions like data_init are not under here.
    - `MPI_X` - You will also see MPI regions in the calltree if using the appropriate MPI profiling configuration (see **Builds/**). Examples shown below.

All functions will be called from `main` and most will be grouped under either `comm` or `comp` regions, representing communication and computation, respectively. You should be timing as many significant functions in your code as possible. **Do not** time print statements or other insignificant operations that may skew the performance measurements.

### **Nesting Code Regions Example** - all computation code regions should be nested in the "comp" parent code region as following:
```
CALI_MARK_BEGIN("comp");
CALI_MARK_BEGIN("comp_small");
sort_pivots(pivot_arr);
CALI_MARK_END("comp_small");
CALI_MARK_END("comp");

# Other non-computation code
...

CALI_MARK_BEGIN("comp");
CALI_MARK_BEGIN("comp_large");
sort_values(arr);
CALI_MARK_END("comp_large");
CALI_MARK_END("comp");
```

### **Calltree Example**:
```
# MPI Mergesort
4.695 main
├─ 0.001 MPI_Comm_dup
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Finalized
├─ 0.000 MPI_Init
├─ 0.000 MPI_Initialized
├─ 2.599 comm
│  ├─ 2.572 MPI_Barrier
│  └─ 0.027 comm_large
│     ├─ 0.011 MPI_Gather
│     └─ 0.016 MPI_Scatter
├─ 0.910 comp
│  └─ 0.909 comp_large
├─ 0.201 data_init_runtime
└─ 0.440 correctness_check
```


Bitonic Calltree:
```
1.720 main
├─ 0.000 MPI_Init
├─ 0.000 data_init_runtime
├─ 0.026 comp
│  ├─ 0.006 comp_small
│  └─ 0.079 comp_large
├─ 0.006 comm
│  └─ 0.006 MPI_Gather
├─ 0.000 correctness_check
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Initialized
├─ 0.000 MPI_Finalized
└─ 0.080 MPI_Comm_dup
```

Sample Sort Calltree:

```
30.491 main
├─ 0.000 MPI_Init
├─ 0.228 data_init_runtime
├─ 7.454 comp
│  ├─ 3.980 comp_small
│  └─ 3.474 comp_large
├─ 1.377 comm
│  ├─ 0.021 comm_small
│  │  ├─ 0.004 MPI_Gather
│  │  └─ 0.016 MPI_Bcast
│  └─ 1.356 comm_large
│     ├─ 0.015 MPI_Alltoall
│     └─ 0.222 MPI_Alltoallv
├─ 0.115 correctness_check
├─ 0.000 MPI_Finalize
├─ 0.000 MPI_Initialized
├─ 0.000 MPI_Finalized
└─ 19.820 MPI_Comm_dup

```


### 3b. Collect Metadata

Have the following code in your programs to collect metadata:
```
adiak::init(NULL);
adiak::launchdate();    // launch date of the job
adiak::libraries();     // Libraries used
adiak::cmdline();       // Command line used to launch the job
adiak::clustername();   // Name of the cluster
adiak::value("algorithm", algorithm); // The name of the algorithm you are using (e.g., "merge", "bitonic")
adiak::value("programming_model", programming_model); // e.g. "mpi"
adiak::value("data_type", data_type); // The datatype of input elements (e.g., double, int, float)
adiak::value("size_of_data_type", size_of_data_type); // sizeof(datatype) of input elements in bytes (e.g., 1, 2, 4)
adiak::value("input_size", input_size); // The number of elements in input dataset (1000)
adiak::value("input_type", input_type); // For sorting, this would be choices: ("Sorted", "ReverseSorted", "Random", "1_perc_perturbed")
adiak::value("num_procs", num_procs); // The number of processors (MPI ranks)
adiak::value("scalability", scalability); // The scalability of your algorithm. choices: ("strong", "weak")
adiak::value("group_num", group_number); // The number of your group (integer, e.g., 1, 10)
adiak::value("implementation_source", implementation_source); // Where you got the source code of your algorithm. choices: ("online", "ai", "handwritten").
```


Bitonic Metadata:


Sample Sort Metadata:

```
 adiak::init(NULL);
 adiak::launchdate();
 adiak::libraries();
 adiak::cmdline();
 adiak::clustername();
 adiak::value("algorithm", "sample_sort");
 adiak::value("programming_model", "mpi");
 adiak::value("data_type", "int");
 adiak::value("size_of_data_type", 4);
 adiak::value("input_size", 268435456);
 adiak::value("input_type", Random);
 adiak::value("num_tasks", 32); 
 adiak::value("scalability", "strong"); 
 adiak::value("group_num", "20"); 
 adiak::value("implementation_source", "handwritten"); 

```
They will show up in the `Thicket.metadata` if the caliper file is read into Thicket.

### **See the `Builds/` directory to find the correct Caliper configurations to get the performance metrics.** They will show up in the `Thicket.dataframe` when the Caliper file is read into Thicket.

## Algorithm Descriptions: 

- Sample Sort Algorithm:
```
    Helper Functions:

    1. void quicksort(std::vector<int>& arr);
        - Sorts the array given as an argument using std::sort. 

    2. std::vector<int> generateSortedData(int size);
        - Creates a vector called data.
        - Through a for loop adds values from 0 to inputted size
        to create a sorted array and returns it. 

    3. std::vector<int> generatePerturbedData(int size);
        - Creates a vector called data using the generateSortedData function.
        - Calculates 1% of the size given. 
        - For 1% of the total size of values swap random elements in the
        array to generate perturbed data.

    4. std::vector<int> generateRandomData(int size);
        - Creates a vector called data
        - Through a for loop from index 0 to inputted_size
        generates random data and adds it to vector.

    5. std::vector<int> generateReverseSortedData(int size);
        - Creates a vector called data.
        - Through a for loop adds values from inputted size - 1 to 0
        to create a reversed sorted array and returns it. 

    6. std::vector<int> generateRandomInput(int size, const std::string& input_type)
        - Uses the given input_type to decide which generation function to use.
        - Calls data generation function based on input_type. 
          If the input_type = "Random", it will call generateRandomData(int size).
    
    Main Function:

    1. Initalize MPI
        The algorithm starts by initializing the MPI environment 
        using MPI_Init and setting up task ids for each process 
        and the total number of processors (num_tasks).
        Adiak is then initialized to collect metadata about the execution environment.

    2. Set variables from job script arguments 
        The input parameters passed to the program are extracted:
            input_size: Total number of elements in the array to be sorted.
            num_tasks: Number of processors.
            input_type: The type of data input (sorted, random, etc.).
    
    3. Ensure there are at least two processors
        If statement to make sure num_tasks is at least 2. If not,
        then the program will abort. 

    4. Adiak collects various data about the program 
        This includes algorithm, input_type, etc. 

    5. Generate data for sorting 
        We start off by marking the region of init with caliper for timing.
        A vector for local_data is created to generate an array based 
        off of the input type. Each processor will receive a certain 
        size of data to generate. This will be evenly divided. 
        For example, if there are 8 processors, each processor 
        generates input_size / 8 elements based on the specified input_type.
    
    6. Sort local data 
        Each processor will sort the data that it created.
        This will once again be timed by caliper but 
        it will be noted as a small computation since we 
        are only sorting local data and not the whole array.
    
    7. Drawing sample size 
        Each processor will draw a sample size to determine how many values to sample.
        The size s is computed as the logarithm of the number of local data 
        elements (log2(local_data.size())). The purpose of this is to reduce
        communication overhead by ensuring that the sample size will be more consistent
        to the size of the data. Based on this sample size, each processor will 
        randomly pick s samples from its data. 
    
    8. Gathering samples 
        All processors send their local samples to a designated root process 
        using MPI_Gather. MPI_Gather will collect the samples from all 
        processors and send them to a designated root processor. 
        The root processor receives all the samples and will combine
        them into a single array. This process will be marked and timed
        as a small communication by caliper. 
    
    9. Sample Sorting and Splitting 
        The root process now sorts the samples. It then selects num_tasks - 1 splitters 
        from the sorted samples, which will be used to partition the data 
        into buckets across all processors. Therefore the number of buckets and 
        the number of processors should be equivalent. The formula s * (i + 1) is used
        to secure indices ensuring a good distance that efficiently partions the data.
        The splitters are then broadcast to all processors using MPI_Bcast. 
        This ensures that every processor knows the ranges of each bucket. Sorting
        the samples is marked and timed as a small computation by caliper, while the 
        Bcast function will be marked and timed as a small communication. 

    10. Putting data into buckets 
         We create a 2D vector, buckets, where each inner vector represents a bucket. 
         The number of buckets is equal to the number of processors. 
         Each processor has its own bucket where data will be partitioned. 
         Each inner vector (buckets[i]) holds the values that will
         be sent to processor i during the data exchange phase. The code iterates 
         over each value in the local data. Each value will be placed into one of the buckets 
         based on the splitters. This loop goes through each splitter 
         to determine where the current value should be placed. The loop checks if the value 
         is less than or equal to the current splitter. If less, the bucket index 
         is set to i, meaning the value belongs in the bucket for that splitter. 
         The loop then breaks as the correct bucket is found. If the value is 
         greater than the current splitter, the code moves on to 
         check the next splitter and increments.

    1l. Assigning buckets to processors 
         We then initialize our counts. We will use these
         to assign sizes of each buckets to sendcounts to tell MPI
         how much each processor will be sending to other processors
         and receiving from other processors. Using all to all, we 
         send these values to all of the processors. Next we 
         calculate the total size of data the current processor will receive
         by summing up all elements in recvcounts and initialize
         recvbuf to hold the data. The send_index and 
         receive_index arrays are then set up to indicate the starting positions 
         for sending and receiving data in the buffers for each processor. 
         We then convert the 2D buckets vector into a single sendbuf 
         vector so that it can be processed by MPI, enabling each processor 
         to send its data to the appropriate destination.

    12. Sending data to buckets and sorting data 
         Using MPI_Alltoallv, each processor sends its data in each bucket 
         to the appropriate processors. This and steps 10-11 are marked as a 
         large communication and timed by caliper. We then mark a large computation
         to time the sorting of all arrays by the processors. After receiving the 
         partitioned data, each processor sorts the received data locally.

    13. Correctness Check
         After sorting is completed, the alorithm checks if the locally
         sorted data is in order using std::is_sorted. If not, an error is called.

    14. Finalizing MPI
        The MPI environment is finalized, marking the end of the program.

```
- Merge Sort Algorithm:
```
	Sorting Functions:
	1. void merge(int *arr1, int *arr2, int LI, int mid, int RI):
		- This function allows for combination of two sorted subarrays into one sorted array.
		- It takes the left index, midpoint, right index to find the boundaries of each subarray within the original array.
		- As we go through each of the index for the left or right, we check if either the left or the right value in the each subarray is smaller before deciding which to copy into the temporary array.
		- If we have any leftover elements after the indexes are hit their respective conditions, it will copy those to the temporary array which is arr2 in this case.
		- Following the subarray sorting, the temporary array will be copied into arr1.
	2. void mergeSort(int *arr1, int *arr2, int LI, int RI):
		- This function will recursively divide the array into smaller subarrays before sorting them and merging them back sorted.
		- As long as the left index is less than the right index  which makes sure it has enough elements to sort (at least 2 elements), we would need to find the middle point.
		- This leads to the recursive call of mergeSort which allows it to keep dividing up the subarray for both left and the right sides in the subarray.
		- After the dividing is complete, the merging will take place where it will call the above merge function to merge the subarrays in a sorted manner.

	Main Function:
	1. Initialize MPI/Adiak
		- Begin by initializing the MPI environment and retrieving the rank of the current process with taskid as well as the total number of processes with numtasks
		- Initialize Adiak as well for performance monitoring and logging as well as to collect metadata 
	2. Input Validation
		- Ensure that there is three command-line arguments.
	3. Declare and Initialize Variables:
		- Read and convert the command-line arguments into its respective variables.
		- These arguments are the array size, number of processes, and the input type.
		- Make sure there are at least 2 MPI processes running in the program as the program needs multiple processes to distribute work.
	4. Collect Data with Adiak:
		- Collect the needed information with Adiak: algorithm, programming model, data type…
	5. Array Allocation:
		- Allocate memory for the original array based on the given input size in preparation for the population of the array 
	6. Array Population with Input Type:
		- Populate the original array with the input types: sorted, random, reverse, 1%perturbed.
		- Sorted: provide a basic sorted array based on the size of the array
		- Random: randomly generate numbers for the array
		- Reverse: provide a basic reversed array based on the size of the array
		- 1%perturbed: randomly select a few values and determine the amount of swaps 
		* this also marks the end of data initialization
	7. Subarray Size and Memory:
		- Determine each subarray size for process distribution
		- Allocate memory for the subarray to be able to hold data for the current process
	8. Scatter
		- Start tracking comm and comm_large performance
		- Use a barrier to synchronize all the processes in the communicator
		- Distribute subarrays from the original array to each of the processes
	9. Merge Sort
		- Start tracking comp and comp_large performance
		- Allocate a temporary memory for the tempArray that will be used for sorting in each process.
		- Call the mergeSort function to sort that subArray
	10. Allocate memory for final merge:
		- Create a pointer to be used for the final sorted array 
		- In the master process, we want to allocate memory for the final sorted array
	11. Gather:
		- Start tracking comm and comm_large performance
		- Use MPI_Gather to collect all the sorted subarrays from all the processes into the master process.
	12. Final Merge:
		- Make sure its all the master process before allocating another temporary memory for the new temporary array that will be used in the mergeSort process for the final sorting.
		- Start tracking comp and comp_large performance
		- Call the mergeSort function for the sorting of the final array
	13. Correctness Check:
		- Start correctness_check to measure performance
		- Use a boolean to check if its sorted with the is_sorted function. 
		- If the boolean is true then it has been sorted and will output into the file that “The data is sorted”. 
	14. Finalize and Cleanup
		- Free all memory allocated areas
	 	- Call MPI_Finalize() to finalize the MPI environment, finishing the program.
```

## 4. Performance evaluation

Include detailed analysis of computation performance, communication performance. 
Include figures and explanation of your analysis.

### 4a. Vary the following parameters
For input_size's:
- 2^16, 2^18, 2^20, 2^22, 2^24, 2^26, 2^28

For input_type's:
- Sorted, Random, Reverse sorted, 1%perturbed

MPI: num_procs:
- 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024

This should result in 4x7x10=280 Caliper files for your MPI experiments.

### 4b. Hints for performance analysis

To automate running a set of experiments, parameterize your program.

- input_type: "Sorted" could generate a sorted input to pass into your algorithms
- algorithm: You can have a switch statement that calls the different algorithms and sets the Adiak variables accordingly
- num_procs: How many MPI ranks you are using

When your program works with these parameters, you can write a shell script 
that will run a for loop over the parameters above (e.g., on 64 processors, 
perform runs that invoke algorithm2 for Sorted, ReverseSorted, and Random data).  

### 4c. You should measure the following performance metrics
- `Time`
    - Min time/rank
    - Max time/rank
    - Avg time/rank
    - Total time
    - Variance time/rank


## 5. Presentation
Plots for the presentation should be as follows:
- For each implementation:
    - For each of comp_large, comm, and main:
        - Strong scaling plots for each input_size with lines for input_type (7 plots - 4 lines each)
        - Strong scaling speedup plot for each input_type (4 plots)
        - Weak scaling plots for each input_type (4 plots)

Analyze these plots and choose a subset to present and explain in your presentation.

## 6. Final Report
Submit a zip named `TeamX.zip` where `X` is your team number. The zip should contain the following files:
- Algorithms: Directory of source code of your algorithms.
- Data: All `.cali` files used to generate the plots seperated by algorithm/implementation.
- Jupyter notebook: The Jupyter notebook(s) used to generate the plots for the report.
- Report.md
