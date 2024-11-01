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
#include <cmath>
#include <random>

// Function to generate sorted data
std::vector<int> generateSortedData(int size) {
    std::vector<int> data(size);
    for (int i = 0; i < size; ++i) {
        data[i] = i;
    }
    return data;
}

// Function to generate 1% perturbed sorted data
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
    std::generate(data.begin(), data.end(), [size]() { return rand() % size; });
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
    if (input_type == "Sorted") return generateSortedData(size);
    else if (input_type == "1%_perturbed") return generatePerturbedData(size);
    else if (input_type == "Random") return generateRandomData(size);
    else if (input_type == "ReverseSorted") return generateReverseSortedData(size);
    throw std::runtime_error("Invalid input type");
}

int main(int argc, char** argv) {
    CALI_CXX_MARK_FUNCTION;

    MPI_Init(&argc, &argv);

    int task_id, num_tasks;
    MPI_Comm_rank(MPI_COMM_WORLD, &task_id);
    MPI_Comm_size(MPI_COMM_WORLD, &num_tasks);

    if (argc < 3) {
        if (task_id == 0) {
            printf("Usage: %s input_size input_type\n", argv[0]);
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
        exit(1);
    }

    int input_size = std::stoi(argv[1]);
    std::string input_type = argv[2];
    if (num_tasks < 2) {
        if (task_id == 0) {
            printf("Need at least two MPI tasks. Quitting...\n");
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
        exit(1);
    }

    adiak::init(NULL);
    adiak::value("algorithm", "radix_sort");
    adiak::value("programming_model", "mpi");
    adiak::value("input_size", input_size);
    adiak::value("input_type", input_type);
    adiak::value("num_tasks", num_tasks);

    // Main is initi
    CALI_MARK_BEGIN("main");

    // Initialization region
    CALI_MARK_BEGIN("data_init_runtime");
    std::vector<int> local_data = generateRandomInput(input_size / num_tasks, input_type);
    CALI_MARK_END("data_init_runtime");

    int bits_per_pass = static_cast<int>(std::ceil(std::log2(num_tasks)));
    int radix = 1 << bits_per_pass;
    int total_passes = (32 + bits_per_pass - 1) / bits_per_pass;

    // Main computation region
    for (int pass = 0; pass < total_passes; ++pass) {
        CALI_MARK_BEGIN("comp");

        // Small computation: calculating destinations for data
        CALI_MARK_BEGIN("comp_small");

        std::vector<int> sendcounts(num_tasks, 0);
        std::vector<std::vector<int>> send_data(num_tasks);
        
        for (int value : local_data) {
            int bits = (value >> (pass * bits_per_pass)) & (radix - 1);
            int target_proc = bits * num_tasks / radix;
            target_proc = (target_proc >= num_tasks) ? num_tasks - 1 : target_proc;

            send_data[target_proc].push_back(value);
            sendcounts[target_proc]++;
        }

        CALI_MARK_END("comp_small");

        // Large computation: preparing flattened send buffer
        CALI_MARK_BEGIN("comp_large");

        std::vector<int> sendbuf;
        std::vector<int> send_displs(num_tasks, 0);
        send_displs[0] = 0;
        for (int i = 0; i < num_tasks; ++i) {
            if (i > 0) {
                send_displs[i] = send_displs[i - 1] + sendcounts[i - 1];
            }
            sendbuf.insert(sendbuf.end(), send_data[i].begin(), send_data[i].end());
        }

        CALI_MARK_END("comp_large");

        CALI_MARK_END("comp");

        // Communication region
        CALI_MARK_BEGIN("comm");

        // Small communication: exchanging counts
        CALI_MARK_BEGIN("comm_small");

        std::vector<int> recvcounts(num_tasks);
        MPI_Alltoall(sendcounts.data(), 1, MPI_INT, recvcounts.data(), 1, MPI_INT, MPI_COMM_WORLD);

        CALI_MARK_END("comm_small");

        // Large communication: exchanging data
        CALI_MARK_BEGIN("comm_large");

        std::vector<int> recv_displs(num_tasks, 0);
        int total_recv_size = recvcounts[0];
        for (int i = 1; i < num_tasks; ++i) {
            recv_displs[i] = recv_displs[i - 1] + recvcounts[i - 1];
            total_recv_size += recvcounts[i];
        }
        std::vector<int> recvbuf(total_recv_size);

        MPI_Alltoallv(sendbuf.data(), sendcounts.data(), send_displs.data(), MPI_INT,
                      recvbuf.data(), recvcounts.data(), recv_displs.data(), MPI_INT, MPI_COMM_WORLD);

        CALI_MARK_END("comm_large");

        CALI_MARK_END("comm");

        // Update local_data with received data for next pass
        local_data = recvbuf;
    }

    // Correctness check outside of computation and communication regions
    CALI_MARK_BEGIN("correctness_check");

    bool sorted = std::is_sorted(local_data.begin(), local_data.end());
    if (!sorted && task_id == 0) {
        std::cerr << "Data not sorted!" << std::endl;
    }

    CALI_MARK_END("correctness_check");
    CALI_MARK_END("main");

    MPI_Finalize();
    return 0;
}
