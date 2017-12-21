# MPI: Sparse Matrix Transpose

## Introductions
Given a linked list with only the non-zero elements of a sparse matrix and their position in the matrix (the row and the column), creates and outputs the transpose of the original matrix. There will be only one process with rank 0, that will read the file name with the input data, read from the file the linked list. 

## Requirements
- C
- Docker
- MPI Image

## General Procedures
1. Create the linked list, given 3 integer, its value and its coordinates in the matrix
2. Determine the dimensions of the matrix based off of the max of the coordinates i and j
3. Create the original matrix
4. Perform Scatterv, to help facilitate the transpose operation via processes 
5. Once transpose is completed, gather the results
6. Print the Matrix Transpose


## How to Run
1. Go to 





