# MergeSort_MPI

### Requirements: 
* CMake >3.12
* C++20
* g++ and gcc 11
* OpenMPI

It is a simple example of parallel programming in MPI. Tests of sorting 100 000 000 real numbers on 4-core processor:

| Number of threads | MPI | One core | Default sort |
| :----------:| --- | -------- | ------------ |
| 1           | 38777 | 27483 | 14231 |
| 2 | 21834 | - | - |
| 3 | 17808 | - | - |
| 4 | 16690 | - | - |
