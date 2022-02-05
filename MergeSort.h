#include <vector>

namespace STSL
{

    template <typename T>
    class Sort
    {
        static std::vector<T> ScatterVectorToThreads(std::vector<T> &data, int rank, int size);
        static std::vector<T> GatherVectorFromThreads(std::vector<T> &data, int rank, int size);
        static std::vector<T> ConcatenateTwoParts(std::vector<T> &partOne, std::vector<T> &partTwo);
        static std::vector<T> MergeSortOnOneCore(std::vector<T> &data, int begin, int end);

    public:
        static std::vector<T> MergeSort(std::vector<T> &data, int rank, int size);
    };

    template <typename T>
    std::vector<T> Sort<T>::MergeSort(std::vector<T> &data, int rank, int size)
    {
        data = ScatterVectorToThreads(data, rank, size);
        data = MergeSortOnOneCore(data, 0, data.size());
        data = GatherVectorFromThreads(data, rank, size);

        return data;
    }

    template <typename T>
    std::vector<T> Sort<T>::ScatterVectorToThreads(std::vector<T> &data, int rank, int size)
    {
        std::vector<T> partToSort;
        
        if (rank == 0)
        {
            // If it is thread number 0, send the appropriate vector parts. 

            int vectorSize = data.size() / size;
            int begin = 0;
            
            partToSort = std::vector<T>(data.begin() + begin, data.begin() + begin + vectorSize);

            begin += vectorSize;
            for (int j = 1; j < size - 1; j++)
            {
                // First, send the size of the vector 
                MPI_Send(&vectorSize, 1, MPI_INT, j, 1, MPI_COMM_WORLD);

                // Second, send just the vector 
                std::vector<T> toSend(data.begin() + begin, data.begin() + begin + vectorSize);
                MPI_Send(&toSend.front(), toSend.size() * sizeof(T), MPI_BYTE, j, 0, MPI_COMM_WORLD);

                // Finally, move to the next one to send 
                begin += vectorSize;
            }

            // Last one can be different size. 
            int lastVectorSize = data.size() - begin;
            MPI_Send(&lastVectorSize, 1, MPI_INT, size - 1, 1, MPI_COMM_WORLD);
            std::vector<T> toSend(data.begin() + begin, data.end());
            MPI_Send(&toSend.front(), toSend.size() * sizeof(T), MPI_BYTE, size - 1, 0, MPI_COMM_WORLD);
        }
        else
        {
            // You are not thread 0, so get data from him 
            MPI_Status status;
            int vectorSize = 0;
            MPI_Recv(&vectorSize, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, &status);

            partToSort = std::vector<T>(vectorSize);
            MPI_Recv(&partToSort.front(), vectorSize * sizeof(T), MPI_BYTE, 0, 0, MPI_COMM_WORLD, &status);
        }

        return partToSort;
    }

    

    template <typename T>
    std::vector<T> Sort<T>::GatherVectorFromThreads(std::vector<T> &data, int rank, int size)
    {
        // In first step it will concatenate results from every 2 threads, later every 4 threads until all are merged. e.g.
        // Step | Threads:
        // 0 | 0 1 2 3 4 5 6
        //     |/  |/  |/  |
        // 1 | 01  23  45  6
        //     |  /    |  /
        // 2 | 0123    456
        //     |     /
        // 3 | 0123456

        int i = 1;
        while (i < size)
        {
            for (int j = 0; j < size; j += 2 * i)
            {
                if ((rank == j || rank == j + i) && j + i < size)
                {
                    if (rank == j)
                    {
                        std::vector<T> partOne = data;
                        std::vector<T> partTwo;

                        // Recieve od j + i
                        MPI_Status status;
                        int sizeOfPartTwo = 0;
                        MPI_Recv(&sizeOfPartTwo, 1, MPI_INT, j + i, 1, MPI_COMM_WORLD, &status);
                        partTwo = std::vector<T>(sizeOfPartTwo);
                        MPI_Recv(&partTwo.front(), sizeOfPartTwo * sizeof(T), MPI_BYTE, j + i, 0, MPI_COMM_WORLD, &status);

                        // Merge these two parts
                        data = ConcatenateTwoParts(partOne, partTwo);
                    }
                    else
                    {
                        // Send to j
                        int sizeOfSendingVector = data.size();
                        MPI_Send(&sizeOfSendingVector, 1, MPI_INT, j, 1, MPI_COMM_WORLD);
                        MPI_Send(&data.front(), data.size() * sizeof(T), MPI_BYTE, j, 0, MPI_COMM_WORLD);
                    }
                }
            }
            i *= 2;
            MPI_Barrier(MPI_COMM_WORLD);
        }

        return data;
    }

    template <typename T>
    std::vector<T> Sort<T>::ConcatenateTwoParts(std::vector<T> &partOne, std::vector<T> &partTwo)
    {
        std::vector<T> result;

        int i = 0;
        int j = 0;
        while (i < partOne.size() || j < partTwo.size())
        {
            if (i < partOne.size() && j < partTwo.size())
            {
                if (partOne[i] < partTwo[j])
                    result.push_back(partOne[i++]);
                else
                    result.push_back(partTwo[j++]);
            }
            else if (i < partOne.size())
            {
                result.push_back(partOne[i++]);
            }
            else
            {
                result.push_back(partTwo[j++]);
            }
        }

        return result;
    }

    template <typename T>
    std::vector<T> Sort<T>::MergeSortOnOneCore(std::vector<T> &data, int begin, int end)
    {
        std::vector<T> result;
        if (begin + 1 == end)
        {
            result.push_back(data[begin]);
            return result;
        }

        int middle = (begin + end) / 2;
        std::vector<T> partOne = MergeSortOnOneCore(data, begin, middle);
        std::vector<T> partTwo = MergeSortOnOneCore(data, middle, end);

        result = ConcatenateTwoParts(partOne, partTwo);

        return result;
    }
}