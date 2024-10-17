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
#include <cmath>

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
    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

    adiak::init(NULL);
    adiak::launchdate();
    adiak::libraries();
    adiak::cmdline();
    adiak::clustername();

    if (argc < 3) {
        if (task_id == 0) {
            printf("Usage: %s input_size input_type\n", argv[0]);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
        exit(1);
    }

    int input_size = std::stoi(argv[1]);   // array size
    std::string input_type = argv[2];      // input type (sorted, random, etc.)

    if (num_tasks < 2) {
        if (task_id == 0) {
            printf("Need at least two MPI tasks. Quitting...\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
        exit(1);
    }

    adiak::value("algorithm", "radix_sort");
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

    // Compute radix sort parameters
    int bits_per_pass = static_cast<int>(std::ceil(std::log2(num_tasks)));
    int radix = 1 << bits_per_pass;
    int total_passes = (32 + bits_per_pass - 1) / bits_per_pass;

    // Start computation
    CALI_MARK_BEGIN("comp");

    for (int pass = 0; pass < total_passes; ++pass) {
        // Computation: counting and preparing data
        CALI_MARK_BEGIN("comp_small");

        // Initialize sendcounts and send_data
        std::vector<int> sendcounts(num_tasks, 0);
        std::vector<std::vector<int>> send_data(num_tasks);

        for (int value : local_data) {
            int bits = (value >> (pass * bits_per_pass)) & (radix - 1);
            int target_proc = bits * num_tasks / radix;

            if (target_proc >= num_tasks) {
                target_proc = num_tasks - 1;
            }

            send_data[target_proc].push_back(value);
            sendcounts[target_proc]++;
        }

        // Flatten send_data into sendbuf
        std::vector<int> sendbuf;
        std::vector<int> send_displs(num_tasks, 0);

        send_displs[0] = 0;
        for (int i = 0; i < num_tasks; ++i) {
            if (i > 0) {
                send_displs[i] = send_displs[i - 1] + sendcounts[i - 1];
            }
            sendbuf.insert(sendbuf.end(), send_data[i].begin(), send_data[i].end());
        }

        CALI_MARK_END("comp_small");

        // Communication: exchange counts and data
        CALI_MARK_BEGIN("comm");

        // First, exchange counts
        CALI_MARK_BEGIN("comm_small");

        std::vector<int> recvcounts(num_tasks);

        MPI_Alltoall(sendcounts.data(), 1, MPI_INT, recvcounts.data(), 1, MPI_INT, MPI_COMM_WORLD);

        // Compute recv_displs
        std::vector<int> recv_displs(num_tasks, 0);
        recv_displs[0] = 0;
        int total_recv_size = recvcounts[0];

        for (int i = 1; i < num_tasks; ++i) {
            recv_displs[i] = recv_displs[i - 1] + recvcounts[i - 1];
            total_recv_size += recvcounts[i];
        }

        std::vector<int> recvbuf(total_recv_size);

        CALI_MARK_END("comm_small");

        CALI_MARK_BEGIN("comm_large");

        MPI_Alltoallv(sendbuf.data(), sendcounts.data(), send_displs.data(), MPI_INT,
                      recvbuf.data(), recvcounts.data(), recv_displs.data(), MPI_INT, MPI_COMM_WORLD);

        CALI_MARK_END("comm_large");

        CALI_MARK_END("comm");

        // Update local_data with received data
        local_data = recvbuf;
    }

    CALI_MARK_END("comp");

    // Correctness check
    CALI_MARK_BEGIN("correctness_check");

    bool sorted = std::is_sorted(local_data.begin(), local_data.end());
    if (!sorted) {
        std::cerr << "Process " << task_id << ": The data is not sorted!" << std::endl;
    }

    CALI_MARK_END("correctness_check");

    MPI_Finalize();

    return 0;
}
