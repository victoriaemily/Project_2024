# CSCE 435 Group project

## 0. Group number: 

## 1. Group members:
1. First
2. Second
3. Third
4. Fourth

## 2. Project topic (e.g., parallel sorting algorithms)

### 2a. Brief project description (what algorithms will you be comparing and on what architectures)

- Bitonic Sort:
- Sample Sort:
- Merge Sort:
- Radix Sort:

### 2b. Pseudocode for each parallel algorithm
- For MPI programs, include MPI calls you will use to coordinate between processes

### 2c. Evaluation plan - what and how will you measure and compare
- Input sizes, Input types
- Strong scaling (same problem size, increase number of processors/nodes)
- Weak scaling (increase problem size, increase number of processors)

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

They will show up in the `Thicket.metadata` if the caliper file is read into Thicket.

### **See the `Builds/` directory to find the correct Caliper configurations to get the performance metrics.** They will show up in the `Thicket.dataframe` when the Caliper file is read into Thicket.

## Algorithm Descriptions: 

```
- Sample Sort Algorithm:

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
