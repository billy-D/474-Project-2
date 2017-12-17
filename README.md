# MPI: Matrix Transpose

## Introductions
Given a very large (n,m>10) matrix of n x m elements, write a distributed program in MPI that creates and outputs the transpose of the original matrix. There will be only one process with rank 0, that will read the file name with the input data, read from the file the values of n and m, read from the file the entire matrix, and will output the resulting matrix on the screen. All the other processes will receive portions of the matrix and contribute to creating the transposed matrix.

## Requirements
- C++
- Docker
- MPI Image

## Procedures (sketch)
1. Create the linked list, given a integer value and its coordinates and the matrix
2. Determine the dimensions of the matrix based off of the max of the coordinates i and j
3. Create the original matrix
4. Transpose the original matrix
5. 


