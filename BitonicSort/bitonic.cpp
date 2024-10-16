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
#include <iterator>

// Quicksort function to sort each bucket (used for local sorting in bitonic sort)
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

// Function to generate input data based on type
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

// Function to compare and swap values based on bitonic sorting direction
void compare_and_swap(std::vector<int>& data, int i, int j, bool ascending) {
    if ((data[i] > data[j]) == ascending) {
        std::swap(data[i], data[j]);
    }
}

// Recursive bitonic merge function
void bitonic_merge(std::vector<int>& data, int low, int cnt, bool ascending) {
    if (cnt > 1) {
        int k = cnt / 2;
        for (int i = low; i < low + k; ++i) {
            compare_and_swap(data, i, i + k, ascending);
        }
        bitonic_merge(data, low, k, ascending);
        bitonic_merge(data, low + k, k, ascending);
    }
}

// Function to perform bitonic sort
void bitonic_sort(std::vector<int>& data, int low, int cnt, bool ascending) {
    if (cnt > 1) {
        int k = cnt / 2;
        bitonic_sort(data, low, k, true);
        bitonic_sort(data, low + k, k, false);
        bitonic_merge(data, low, cnt, ascending);
    }
}

int main(int argc, char** argv) {
    CALI_CXX_MARK_FUNCTION;

    MPI_Init(&argc, &argv);

    int task_id;
    int num_tasks;
    int rc;
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

    adiak::value("algorithm", "bitonic_sort");
    adiak::value("programming_model", "mpi");
    adiak::value("data_type", "int");
    adiak::value("size_of_data_type", sizeof(int));
    adiak::value("input_size", input_size);
    adiak::value("input_type", input_type);
    adiak::value("num_procs", num_tasks);
    adiak::value("scalability", "strong");
    adiak::value("group_num", 20);
    adiak::value("implementation_source", "handwritten");

    // Generate random input data based on input type
    CALI_MARK_BEGIN("data_init_runtime");

    std::vector<int> local_data = generateRandomInput(input_size / num_tasks, input_type);

    CALI_MARK_END("data_init_runtime");

    // Computation region (sorting local data)
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_small");

    quicksort(local_data); // Locally sorting data on each process

    CALI_MARK_END("comp_small");
    CALI_MARK_END("comp");

    // Communication region for gathering all data and performing bitonic sort
    CALI_MARK_BEGIN("comm");

    std::vector<int> global_data(input_size);
    MPI_Gather(local_data.data(), local_data.size(), MPI_INT,
               global_data.data(), local_data.size(), MPI_INT,
               0, MPI_COMM_WORLD);

    CALI_MARK_END("comm");

    if (task_id == 0) {
        // Bitonic sorting all gathered data
        CALI_MARK_BEGIN("comp");
        CALI_MARK_BEGIN("comp_large");

        bitonic_sort(global_data, 0, input_size, true);

        CALI_MARK_END("comp_large");
        CALI_MARK_END("comp");
    }

    // Correctness check
    CALI_MARK_BEGIN("correctness_check");
    if (task_id == 0) {
        bool sorted = std::is_sorted(global_data.begin(), global_data.end());
        if (!sorted) {
            std::cerr << "The data is not sorted!" << std::endl;
        }
        else if (sorted) {
          std::cout << "Data is sorted correctly." << std::endl;
        }
    }
    CALI_MARK_END("correctness_check");

    MPI_Finalize();
}
