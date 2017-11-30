/**
 * Author: Chris Anderson
 * Program: Project 2
 * Date: 12/11/2016
 * Description:
 *
 * Compressing a sparse matrix:
 *
 *     Given a very large (n, m>10) matrix of n x m elements, a very large percentage of
 *     them being 0, write a distributed program in MPI that creates and outputs a linked
 *     list with only the non-zero elements and their position in the matrix (the row and
 *     the column). There will be only one process with rank 0, that will read the file
 *     name with the input data, read from the file the values of n and m, read from the
 *     file the entire matrix, and will output the resulting linked list on the screen.
 *     All the other processes will receive portions of the matrix and contribute to the
 *     linked list.
 *
 * Instructions:
 * 1) Create a directory on your host machine and put the following tow files in it; matrix,  Project2.c
 * 2) Run the following docker command from the directory you just created:
 * 		docker run --rm -it -v "${PWD}:/project" nlknguyen/alpine-mpich
 * 3) Compile the program using the following command:
 * 		mpicc -o Project2 Project2.c
 * 4) Run the program with a specified number of processes:
 * 		mpirun -n 10 ./Project2
 *
 * Assumptions:
 * 1) We're only dealing with Integers.
 * 2) The input files name is "matrix" and is in the same directory as the executable.
 * 3) Each row in the file must be less than 100 bytes including spaces.
 * 4) The program must be run with a minimum of 2 processes as process zero is
 *    only used for managing the distribution of work.
 *
 * Issues:
 * 1) The size of the matrix (row, col) is hardcoded as I could not figure out for the life
 * of me how to dynamically malloc a 2D array at runtime.  I was close with the commented
 * out code you'll see below but if any process had to process more than one row it
 * would not see any row after the first.
 *
 *     ex) processing a matrix with 4 rows using 3 processes.
 *
 * 2) I created a chunk_size function based on the para_range function presented in class.
 * I'm not sure what is wrong but the method does not always work, noticibly when the number
 * of processors is significantly less than the number of rows. I ran out of time to
 * troubleshoot it.
 *
 * File Layout:
 * Below is a sample input file.  The first line is the number of rows in the
 * matrix and line two is the number of columns.  The remaining lines are the
 * actual content of the matrix.
 *
 * 4
 * 6
 * 0 2 0 0 0 0
 * 0 0 3 1 0 0
 * 1 0 0 5 4 6
 * 0 0 0 0 0 1
 *
 */

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

struct node{
	int row, col, value;
	struct node *next;
};

/**
 * This method is used to divide up the matrix into chunks to be
 * processed by each thread.  I borrowed upon the para_range method
 * presented in class.
 *
 * It should be noted that this function does not work for all values
 * of size.  I'm not entirely sure what's wrong but after playing with
 * several values it doesn't always work.
 *
 * n1 is the lowest value of iteration variable
 * n2 is the highest value of iteration variable
 * size is # cores
 * rank the rank of the core in the communicator
 */
int chunk_size(int n1, int n2, int size, int rank){
	int iwork1, iwork2, start, end;
	iwork1 = (n2 - n1 + 1) / size;
	iwork2 = ((n2 - n1 + 1) % size);
	start = rank * iwork1 + n1 + min(rank, iwork2);
	end = start + iwork1 - 1;
	if(iwork2 > rank)
		end = end + 1;

	return end - start + 1;
}

//Entry point to application.  No arguments are expected.
int main(int argc, char *argv[]){
	//create common variables
	int row = 8;
//	int row;
	int col = 6;
//	int col;
	int matrix[row][col];
//	int **matrix;
	const char delimiter[1] = " ";
	int maxElements;

	//Initialize MPI and get rank and size
	int rank, size;
	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(size == 1){
		printf("This program must be run with a minimum of 2 processes (-n 2)\n");
		return -1;
	}

	//Keep track of the start time so we can calculate the runtime at the end.
	clock_t begin;

	//Process zero is responsible for reading in the matrix from the file.
	if(rank == 0){
		begin = clock();

		//Open the file for reading
		FILE *fp = fopen("matrix", "r");

		//Read in the size of the matrix
		const int BUFFER = 100;
		char line[BUFFER];

		//Get number of rows out of 100 byte buffer from line one
		fgets(line, BUFFER, fp);
//		sscanf(line, "%d", &row);
		//Get number of columns out of 100 byte buffer from line two
		fgets(line, BUFFER, fp);
//		sscanf(line, "%d", &col);

//		//Dynamically allocate memory to matrix
//		matrix = (int **)malloc(row * sizeof(int *));
//		for(int i = 0; i < row; i++){
//			matrix[i] = (int *)malloc(col * sizeof(int));
//		}
////////READ FROM FILE HERE/////////////////////////
		for(int i = 0; i < row; i++){
			//Read a line up to the newline character
			fgets(line, BUFFER, fp);
			char *p = strtok(line, delimiter);
			int k = 0;
			while(p != NULL){
				matrix[i][k++] = atoi(p);
				p = strtok(NULL, " ");
			}
////END READ FILE HERE/////////////////////////////////
			//Make sure we parsed the supplied number of columns
			if(k != col){
				printf("INVALID ARGUMENTS  k=%d, col=%d", k, col);
				fclose(fp);
				return -1;
			}
		}

		//Close the file
		fclose(fp);

		//Print out the matrix
		printf("\nRows = %d\n", row);
		printf("Cols = %d\n", col);
		printf("-----------------\n");
		for(int i = 0; i < row; i++){
			for(int j = 0; j < col; j++)
				printf("%d ", matrix[i][j]);
			printf("\n");
		}
		printf("\n");
	}

	//Broadcast the matrix dimensions to all processes.
	MPI_Bcast(&row, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&col, 1, MPI_INT, 0, MPI_COMM_WORLD);

	//Determine the chunk size for each process
	int loc_dim = row * col;
	if(rank != 0){
		//Accommodate process 0 not processing any of the matrix rows.
		if(size < row)
			loc_dim = chunk_size(1, row + 2, size, rank);
		else if(size == row)
			loc_dim = chunk_size(1, row + 2, size, rank);
		else
			loc_dim = chunk_size(1, row + 1, size, rank);
	}

	//Setup the sparse array
	int sparseArrayRow[loc_dim * col];
	int sparseArrayCol[loc_dim * col];
	int sparseArrayValue[loc_dim * col];
	int sparseArraySize = 0;

	MPI_Request ireq;
	MPI_Status istatus;

	//Use rank 0 to distribute out the work evenly among processes
	if(rank == 0){
		//Wait for each process to request a certain number of rows to process
		int rowsProcessed = 0;
		for(int i = 1; i < size; i++){
			int numRowsRequested = 0;
			MPI_Irecv(&numRowsRequested, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &ireq);
			MPI_Wait(&ireq, &istatus);

			//If the process requested zero rows because we have more processes than rows,
			//continue onto the next request
			if(numRowsRequested == 0)
			{
				continue;
			}
			else
			{
				//Send back the next chunk of data to be processed based on the requested
				//chunk size.
				MPI_Isend(matrix[rowsProcessed], numRowsRequested * col, MPI_INT, i, 0, MPI_COMM_WORLD, &ireq);
				MPI_Isend(&rowsProcessed, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &ireq);
				rowsProcessed += numRowsRequested;
			}
		}
	}
///////////TRANSPOSE GOES HERE///////////////////////////	
	else
	{
		if(loc_dim == 0)
		{
			//if the loc_dim value is 0 because there's not enough data then just
			//send a request for zero records to the root process knows not to wait.
			MPI_Isend(&loc_dim, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &ireq);
		}
		else{
			//Request the number of rows this process is ready to process.
			MPI_Isend(&loc_dim, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &ireq);

			//Receive back the requested number of rows, along with which which particular
			//row I'm starting with.
			int tempArray[loc_dim][col], startRow = 0;
			MPI_Irecv(&tempArray, loc_dim * col, MPI_INT, 0, 0, MPI_COMM_WORLD, &ireq);
			MPI_Wait(&ireq, &istatus);
			MPI_Irecv(&startRow, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &ireq);
			MPI_Wait(&ireq, &istatus);

			//Add only non-zero values to the sparse array.
			for(int i = 0; i < loc_dim; i++)
			{
				for(int j = 0; j < col; j++)
				{
					int temp;
					temp = row;
					row = col;
					col = temp;
					
				}
			}
		}
	}
/////////////////////////END TRANSPOSE/////////////////////////////////
	MPI_Barrier(MPI_COMM_WORLD);	//This may not be necessary

	//The parent process will wait for the children to send in their resulting sparse arrays
	//so that it can consolidate them into a single list.
	if(rank == 0){
		int numRows;

		//x are the processes
		int x = 0;
		if(size <= row)
			x = size;
		else
			x = row + 1;

		int totalNodes = 0;
		struct node *root;
		struct node *child;
		for(int i = 1; i < x; i++){

			MPI_Irecv(&sparseArrayRow, row * col, MPI_INT, i, 0, MPI_COMM_WORLD, &ireq);
			MPI_Wait(&ireq, &istatus);
			MPI_Get_count(&istatus, MPI_INT, &numRows);
			MPI_Irecv(&sparseArrayCol, row * col, MPI_INT, i, 0, MPI_COMM_WORLD, &ireq);
			MPI_Wait(&ireq, &istatus);
			MPI_Irecv(&sparseArrayValue, row * col, MPI_INT, i, 0, MPI_COMM_WORLD, &ireq);
			MPI_Wait(&ireq, &istatus);

			for(int j = 0; j < numRows; j++){
				int value = sparseArrayValue[j];
				if(totalNodes == 0){
					root = malloc(sizeof(struct node));
					child = root;
				}else{
					child->next = malloc(sizeof(struct node));
					child = child->next;
				}

				child->row = sparseArrayRow[j];
				child->col = sparseArrayCol[j];
				child->value = value;
				child->next = 0;

				totalNodes++;
			}
		}

	    //Print out the linked list
	   for(int i = 0; i < row; i++){
			for(int j = 0; j < col; j++)
				printf("%d ", matrix[i][j]);
			printf("\n");
		}
		printf("\n");
I
		printf("Runtime: %f seconds\n", ((double)(clock() - begin) / CLOCKS_PER_SEC));
	}else if(loc_dim != 0){
		//Send this processes sparse array to the parent process
		MPI_Isend(&sparseArrayRow, sparseArraySize, MPI_INT, 0, 0, MPI_COMM_WORLD, &ireq);
		MPI_Wait(&ireq, &istatus);
		MPI_Isend(&sparseArrayCol, sparseArraySize, MPI_INT, 0, 0, MPI_COMM_WORLD, &ireq);
		MPI_Wait(&ireq, &istatus);
		MPI_Isend(&sparseArrayValue, sparseArraySize, MPI_INT, 0, 0, MPI_COMM_WORLD, &ireq);
		MPI_Wait(&ireq, &istatus);
	}

	MPI_Finalize();

    return 0;
}
