#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <mpi.h>
#include <caliper/cali.h>
#include <adiak.hpp>
#include <numeric>
#include <stdexcept>
#include <string>
#include <random>

// Quicksort function to sort each bucket 
void quicksort(std::vector<int>& arr) {
    std::sort(arr.begin(), arr.end());
}

// Function to generate sorted data
std::vector<int> generateSortedData(int size) {
    std::vector<int> data(size);
    for (int i = 0; i < size; ++i) {
        data[i] = i;
    }
    return data;
}

/* Function to generate 1% perturbed sorted data, calculates 1% of 
the total array size then performs that number of random swaps between elements */
std::vector<int> generatePerturbedData(int size) {
    std::vector<int> data = generateSortedData(size);
    int perturb_size = size / 100;  
    for (int i = 0; i < perturb_size; ++i) {
        int index1 = rand() % size;
        int index2 = rand() % size;
        std::swap(data[index1], data[index2]);
    }
    return data;
}

// Function to generate random data
std::vector<int> generateRandomData(int size) {
    std::vector<int> data(size);
    for (int i = 0; i < size; ++i) {
        data[i] = rand() % size;
    }
    return data;
}

// Function to generate reverse sorted data
std::vector<int> generateReverseSortedData(int size) {
    std::vector<int> data(size);
    for (int i = 0; i < size; ++i) {
        data[i] = size - i;
    }
    return data;
}

// Function to generate random input data based on type
std::vector<int> generateRandomInput(int size, const std::string& input_type) {
    srand(time(0));  

    if (input_type == "Sorted") {
        return generateSortedData(size);
    } else if (input_type == "1%_perturbed") {
        return generatePerturbedData(size);
    } else if (input_type == "Random") {
        return generateRandomData(size);
    } else if (input_type == "ReverseSorted") {
        return generateReverseSortedData(size);
    } else {
        throw std::runtime_error("Invalid input type");
    }
}

int main(int argc, char** argv) {
    CALI_CXX_MARK_FUNCTION; 

    MPI_Init(&argc, &argv);

    int task_id; // task identifier 
    int num_tasks; // number of processors 
    int rc; // return code 
    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

    adiak::init(NULL);
    adiak::launchdate();
    adiak::libraries();
    adiak::cmdline();
    adiak::clustername();

    int input_size = std::stoi(argv[1]);   // array size
    num_tasks = std::stoi(argv[2]);        // num processors
    std::string input_type = argv[3];      // input type (sorted, random, etc.)

    if (num_tasks < 2 ) {
        printf("Need at least two MPI tasks. Quitting...\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        exit(1);
    }

    adiak::value("algorithm", "sample_sort");
    adiak::value("programming_model", "mpi");
    adiak::value("data_type", "int");
    adiak::value("size_of_data_type", sizeof(int));
    adiak::value("input_size", input_size);
    adiak::value("input_type", input_type);
    adiak::value("num_tasks", num_tasks); 
    adiak::value("scalability", "strong"); 
    adiak::value("group_num", "20"); 
    adiak::value("implementation_source", "handwritten"); 

    // Generate random input data based on input type
    CALI_MARK_BEGIN("data_init_runtime");

    std::vector<int> local_data = generateRandomInput(input_size / num_tasks, input_type);

    CALI_MARK_END("data_init_runtime");

    // Computation region
    CALI_MARK_BEGIN("comp");

    // Each processor sorts local data 
    CALI_MARK_BEGIN("comp_small");
    quicksort(local_data);
    CALI_MARK_END("comp_small");

    CALI_MARK_END("comp");

    // Communication region
    CALI_MARK_BEGIN("comm");

    // Drawing sample of size s random samples 
    int s = static_cast<int>(log2(local_data.size())); // Use log to reduce communication overhead 
    std::vector<int> samples(s);
    for (int i = 0; i < s; ++i) {
        samples[i] = local_data[rand() % local_data.size()]; // Picking random values 
    }

    // Communication for sample exchange (only sending a few values)
    CALI_MARK_BEGIN("comm_small");

    std::vector<int> gathered_samples(s * num_tasks); // initialize a vector for total number of samples

    /* MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
    void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm) */
    MPI_Gather(samples.data(), s, MPI_INT, gathered_samples.data(), s, MPI_INT, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_small");

    CALI_MARK_END("comm");

    CALI_MARK_BEGIN("comp");

    // Sort the gathered samples and make a vector with m-1 values 
    std::vector<int> splitters(num_tasks - 1);
    if (task_id == 0) {
        CALI_MARK_BEGIN("comp_small");
        quicksort(gathered_samples);  // Sorting samples
        CALI_MARK_END("comp_small");

        // Choosing m-1 values to be splitters 
        for (int i = 0; i < num_tasks - 1; ++i) {
            splitters[i] = gathered_samples[s * (i + 1)];
        }
    }

    CALI_MARK_END("comp");

    CALI_MARK_BEGIN("comm");

    CALI_MARK_BEGIN("comm_small");
   
    // Broadcasting Splitters so that all processes receive the splitters
    MPI_Bcast(splitters.data(), num_tasks - 1, MPI_INT, 0, MPI_COMM_WORLD);

    CALI_MARK_END("comm_small");

    // Data exchange for bucket communication (large communication)
    CALI_MARK_BEGIN("comm_large");

    // 2D vector, inner vectors represent a bucket 
    std::vector<std::vector<int>> buckets(num_tasks); 
    for (int value : local_data) {
        int bucket_index = 0;  // Initialize to the first bucket

        // Loop through splitters and determine which bucket the value belongs to
        for (int i = 0; i < splitters.size(); ++i) {
            if (value <= splitters[i]) {
                bucket_index = i;  // If the value is less than or equal to the splitter, assign the bucket
                break;             // Once the correct bucket is found, exit the loop
            }
            // If the value is greater than the current splitter, move to the next bucket
            else {
                bucket_index = i + 1;
            }
        }

        // Add the value to the appropriate bucket
        buckets[bucket_index].push_back(value);
    }

    std::vector<int> sendcounts(num_tasks), recvcounts(num_tasks);
    std::vector<int> send_index(num_tasks), receive_index(num_tasks); // Tells where in buffers to start send/recv
    std::vector<int> sendbuf;
    int total_recv_size = 0;

    // Assigning sizes of each bucket to sendcount array 
    for (int i = 0; i < num_tasks; ++i) {
        sendcounts[i] = buckets[i].size(); 
    }

    // Sending the number of counts to each processor 
    MPI_Alltoall(sendcounts.data(), 1, MPI_INT, recvcounts.data(), 1, MPI_INT, MPI_COMM_WORLD);

    // Summing all elements in recvcounts 
    total_recv_size = std::accumulate(recvcounts.begin(), recvcounts.end(), 0);
    std::vector<int> recvbuf(total_recv_size);

    // Start at index 0
    send_index[0] = 0;
    receive_index[0] = 0;

    // Tell MPI where in the send and receive buffers to find or place the data for each processor
    for (int i = 1; i < num_tasks; ++i) {
        send_index[i] = send_index[i - 1] + sendcounts[i - 1];
        receive_index[i] = receive_index[i - 1] + recvcounts[i - 1];
    }

    // Turn buckets into single vector for MPI to process 
    for (const auto& bucket : buckets) {
        sendbuf.insert(sendbuf.end(), bucket.begin(), bucket.end());
    }

    // Each processor will send data to other processors and receive from other processors 
    MPI_Alltoallv(sendbuf.data(), sendcounts.data(), send_index.data(), MPI_INT,
                  recvbuf.data(), recvcounts.data(), receive_index.data(), MPI_INT, MPI_COMM_WORLD);

    CALI_MARK_END("comm_large");

    CALI_MARK_END("comm");


    CALI_MARK_BEGIN("comp");

    // Sorting received data
    CALI_MARK_BEGIN("comp_large");
    quicksort(recvbuf);
    CALI_MARK_END("comp_large");

    CALI_MARK_BEGIN("comp");

    // Correctness check
    CALI_MARK_BEGIN("correctness_check");
    bool sorted = std::is_sorted(recvbuf.begin(), recvbuf.end());
    if (!sorted && task_id == 0) {
        std::cerr << "The data is not sorted!" << std::endl;
    }
    CALI_MARK_END("correctness_check");

    MPI_Finalize();

}
