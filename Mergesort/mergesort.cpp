#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <algorithm>
#include <adiak.hpp>

#include <caliper/cali.h>
#include <caliper/cali-manager.h>
#include <adiak.hpp>

// Merging Arrays Function 
// ** RI is the end index of the right subarray, so from mid + 1 to RI is the right subarray
void merge(int *arr1, int *arr2, int LI, int mid, int RI){
    int LH = LI;  //left subarray index starter
    int i = LI;   //index used to place the merged elements back into arr2
    int RH = mid + 1; //right subarray index starter 
    int j; // used to copy the merged elements back into arr1

    while ((LH <= mid) && (RH <= RI)){
        if (arr1[LH] <= arr1[RH]){  //copy left subaray if smaller
            arr2[i] = arr1[LH];
            LH++;
        }
        else {
            arr2[i] = arr1[RH]; //copy right subarray if smaller
            RH++;
        }
        i++;
    }

    // remaining elements
    if (LH > mid) {
        // no more elements in left, put the remaining elements in the right in arr2
        for (j = RH; j <= RI; j++){
            arr2[i] = arr1[j];
            i++;
        }
    } else {
        for (j = LH; j <= mid; j++){
            arr2[i] = arr1[j];
            i++;
        }
    }

    // after merge: copy elements from temp array arr2 back to original array arr1
    for (j = LI; j <= RI; j++ ){
        arr1[j] = arr2[j];
    }
}

// Merge Sort
void mergeSort(int *arr1, int *arr2, int LI, int RI){
    int mid;

    if (LI < RI){
        mid = (LI + RI) / 2;
        // left side 
        mergeSort(arr1, arr2, LI, mid);
        // right side
        mergeSort(arr1, arr2, mid + 1, RI);
        // final merge
        merge(arr1, arr2, LI, mid, RI);
    }
}

int main(int argc, char *argv[]){

    // array
    // MPIs
    // number of tasks and task identifier
    CALI_CXX_MARK_FUNCTION;

    MPI_Init(&argc,&argv);

    int taskid, numtasks;
    double main_start_time;
    int rc;

    MPI_Comm_rank(MPI_COMM_WORLD,&taskid);
    MPI_Comm_size(MPI_COMM_WORLD,&numtasks);

    adiak::init(NULL);
    adiak::launchdate();
    adiak::libraries();
    adiak::cmdline();
    adiak::clustername();

    if (argc != 4){
        printf("Please provide missing input");
        return 0;
    }

    int sizeOfMatrix = atoi(argv[1]);
    int numProcs = atoi(argv[2]);
    // char* inputType = argv[3];
    std::string inputType = argv[3]; 

    if (numProcs < 2 ) {
        printf("Need at least two MPI tasks. Quitting...\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        exit(1);
    }
   
    adiak::value("algorithm", "merge");
    adiak::value("programming_model", "mpi");
    adiak::value("data_type", "int");
    adiak::value("size_of_data_type", sizeof(int)); 
    adiak::value("input_size", sizeOfMatrix);
    adiak::value("input_type", inputType);
    adiak::value("num_procs", numProcs);
    adiak::value("scalability", "strong");
    adiak::value("group_num", 20); 
    adiak::value("implementation_source", "handwritten"); 

    CALI_MARK_BEGIN("data_init_runtime");
    double data_init_start = MPI_Wtime();

    int *origArray = (int*) malloc(sizeOfMatrix * sizeof(int));
    // specified input type?, populate array ?
    if (inputType == "sorted"){
        for (int i = 0; i < sizeOfMatrix; i++){
            origArray[i] = i;
        }
    }
    else if (inputType == "random"){
        // need to see the random num generator, ensures sequence of random number is different each time based on taskid
        srand(taskid + time(NULL));
        for (int i = 0; i < sizeOfMatrix; i++){
            origArray[i] = rand() % sizeOfMatrix;
        }
    }
    else if (inputType == "reverse"){
        for (int i = 0; i < sizeOfMatrix; i++){
            origArray[i] = sizeOfMatrix - i;
        }
    }
    else if (inputType == "perturbed"){
        for (int i = 0; i < sizeOfMatrix; i++){
            origArray[i] = i;
        } // 1% perturb values of array, increase or decrease 
        // randomly select a few to do so
        // determine amount of swaps by dividing by a 100, especially for 1%
        int perturbAmount = sizeOfMatrix / 100;
        for(int i = 0; i < perturbAmount; i++){
            // making sure that the number will fall in the range, with the modulo
            int i1 = rand() % sizeOfMatrix;
            int i2 = rand() % sizeOfMatrix;
            // swap values
            int tempSwap = origArray[i1];
            origArray[i1] = origArray[i2];
            origArray[i2] = tempSwap;

        }
    } 
    else {
        printf("Invalid input type.");
        MPI_Finalize();
        return 1;
    }
    
    CALI_MARK_END("data_init_runtime"); // change to io?
    double data_init_time = MPI_Wtime() - data_init_start; 
 
    // find the size of each of the sub arrays
    int subSize = sizeOfMatrix / numtasks;
    
    // send the subarrays into each of the processes, divide and conquer
    int *subArr = (int*) malloc(subSize * sizeof(int));

    CALI_MARK_BEGIN("comm");
    MPI_Barrier(MPI_COMM_WORLD);
    CALI_MARK_BEGIN("comm_large");
    double scatter_start = MPI_Wtime();
    MPI_Scatter(origArray, subSize, MPI_INT, subArr, subSize, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");
    double scatter_time= MPI_Wtime() - scatter_start;  

    // start mergesort for each process
    CALI_MARK_BEGIN("comp");
    CALI_MARK_BEGIN("comp_large");
    int *tempArr = (int*) malloc(subSize * sizeof(int));
    double merge_start = MPI_Wtime();
    mergeSort(subArr, tempArr, 0, subSize - 1);
    CALI_MARK_END("comp_large");
    CALI_MARK_END("comp");
    double merge_time = MPI_Wtime() - merge_start;

    // after sorting all the subarrays, put them back together for final sort
    int* sortedSub = NULL;
    // master process needs to have memory for the sorted array 
    if (taskid == 0){
        sortedSub = (int*) malloc(sizeOfMatrix * sizeof(int));
    }
    
    CALI_MARK_BEGIN("comm");
    CALI_MARK_BEGIN("comm_large");
    double gather_start = MPI_Wtime();
    MPI_Gather(subArr, subSize, MPI_INT, sortedSub, subSize, MPI_INT, 0, MPI_COMM_WORLD);
    CALI_MARK_END("comm_large");
    CALI_MARK_END("comm");
    double gather_time = MPI_Wtime() - gather_start;

    // final merge 
    if (taskid == 0){
        int *tempArr2 = (int*) malloc(sizeOfMatrix * sizeof(int));

        CALI_MARK_BEGIN("comp");
        CALI_MARK_BEGIN("comp_large");
        double merge_final_start = MPI_Wtime();

        mergeSort(sortedSub, tempArr2, 0, (sizeOfMatrix-1));
        CALI_MARK_END("comp_large");
        CALI_MARK_END("comp");
        double merge_final_time = MPI_Wtime() - merge_final_start;


        // print out sorted array for check -> change to just check if sort
        CALI_MARK_BEGIN("correctness_check");
        double correctness_check_start = MPI_Wtime();
        bool sorted = std::is_sorted(sortedSub, sortedSub + sizeOfMatrix);
        if (sorted) {
            std::cerr << "The data is sorted" << std::endl;
        } 
       
        // printf("Sorted Array: ");
        // for (int i = 0; i < sizeOfMatrix; i++){
        //     printf("%d ", sortedSub[i]);
        // }
      
        CALI_MARK_END("correctness_check");
        double correctness_check_time = MPI_Wtime() - correctness_check_start;
        
        // speed up calcuations
        // double seq_time = measureSeqSort(origArray, sizeOfMatrix);
        // log metrics
        // log_metrics(sizeOfMatrix, numtasks, seq_time, merge_end - merge_start + (merge_final_end - merge_final_start));

        free(sortedSub);
        free(tempArr2);

    }
    // Flush Caliper output before finalizing MPI
    // finish this 
    free(subArr);
    free(tempArr);
    free(origArray);

    MPI_Finalize();

    // double finalize_time = MPI_Wtime() - finalize_start;
    // printf("MPI Finalize Time: %f seconds\n", finalize_time);
    // mgr.stop();
    // mgr.flush();


}

