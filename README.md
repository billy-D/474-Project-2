# MPI: Sparse Matrix Transpose

## Introductions
Given a linked list with only the non-zero elements of a sparse matrix and their position in the matrix (the row and the column), creates and outputs the transpose of the original matrix. There will be only one process with rank 0, that will read the file name with the input data, read from the file the linked list. 

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


