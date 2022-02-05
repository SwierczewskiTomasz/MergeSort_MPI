#include <iostream>
#include <mpi.h>
#include <vector>
#include <random>
#include <chrono>
#include "MergeSort.h"

std::vector<double> prepareRealVector(int rank, int length, int variation)
{
    std::vector<double> result;

    if (rank == 0)
    {
        std::random_device r;
        std::default_random_engine el(r());
        std::uniform_real_distribution<double> uniformReal(0, variation);

        for (int i = 0; i < length; i++)
        {
            result.push_back(uniformReal(el));
        }
    }

    return result;
}

std::vector<int> prepareIntVector(int rank, int length, int variation)
{
    std::vector<int> result;

    if (rank == 0)
    {
        std::random_device r;
        std::default_random_engine el(r());
        std::uniform_int_distribution<int> uniformDistribution(0, variation);

        for (int i = 0; i < length; i++)
        {
            result.push_back(uniformDistribution(el));
        }
    }

    return result;
}

int main(int argc, char **argv)
{
    int rank, size, len;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::cout << "My rank: " << rank << ". Size: " << size << std::endl;

    // Prepare data to Sort
    // std::vector<int> toSortInt = prepareIntVector(rank, 100000000, 500000000);
    std::vector<double> toSortReal = prepareRealVector(rank, 100000000, 10000);

    MPI_Barrier(MPI_COMM_WORLD);
    auto start = std::chrono::high_resolution_clock::now();

    // std::vector<int> sortedInt = STSL::Sort<int>::MergeSort(toSortInt, rank, size);
    std::vector<double> sortedReal = STSL::Sort<double>::MergeSort(toSortReal, rank, size);

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    MPI_Barrier(MPI_COMM_WORLD);

    std::cout << "My rank: " << rank << ". Time of sorting: " << duration.count() << std::endl;

    MPI_Finalize();
}