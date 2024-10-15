cmake_minimum_required(VERSION 3.12)

# Find required packages
find_package(MPI REQUIRED)
find_package(caliper REQUIRED)
find_package(adiak REQUIRED)

# Change 'mergesort' to your sample sort source file
add_executable(samplesort samplesort.cpp)

# Include directories for MPI, Caliper, and Adiak
message(STATUS "MPI includes : ${MPI_INCLUDE_PATH}")
message(STATUS "Caliper includes : ${caliper_INCLUDE_DIR}")
message(STATUS "Adiak includes : ${adiak_INCLUDE_DIRS}")
include_directories(SYSTEM ${MPI_INCLUDE_PATH})
include_directories(${caliper_INCLUDE_DIR})
include_directories(${adiak_INCLUDE_DIRS})

# Link libraries to the executable
target_link_libraries(samplesort PRIVATE MPI::MPI_CXX)
target_link_libraries(samplesort PRIVATE caliper)
