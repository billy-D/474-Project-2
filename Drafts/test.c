/*--------------------------------------------------------------------
Author: Billy Dang, Sean McKean, Hassan Hamod
Program: Project 2
Date: 12/02/2017
File: 
Description: Sparse Matrix, Matrix Tanspose


Sources: 

Linked List - http://www.zentut.com/c-tutorial/c-linked-list/
 

------------------=-------------------------------------------------*/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })


typedef struct node{
	int i, j, value;
	struct node *next;
} node;

typedef void (*callback)(node* data);

//helper function to help set up the nodes 
node* create(int data,node* next, int row, int col)
{
    node* new_node = (node*)malloc(sizeof(node));
    if(new_node == NULL)
    {
        printf("Error creating a new node.\n");
        exit(0);
    }
    new_node->value = data;
    new_node->i = row;
    new_node->j = col;
    new_node->next = next;
 
    return new_node;
}

//main function to be called in program to start the node making process
node* prepend(node* head,int data, int i, int j)
{
    node* new_node = create(data,head,i,j);
    head = new_node;
    return head;
}


//TEST:print the linked list
void print(node*head)
{
	node* reader = head;
	while(reader != NULL)
	{
		printf("%d ", reader->value);
		printf("%d ", reader->i);
		printf("%d ", reader->j);

		printf("\n");
		reader = reader->next;
	}
}

//Once all operations are done, deallocate memory
void dispose(node *head)
{
    node* cursor, *tmp;
 
    if(head != NULL)
    {
        cursor = head->next;
        head->next = NULL;
        while(cursor != NULL)
        {
            tmp = cursor->next;
            free(cursor);
            cursor = tmp;
        }
    }
}



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
int chunk_size(int n1, int n2, int size, int rank)
{
	int iwork1, iwork2, start, end;
	iwork1 = (n2 - n1 + 1) / size;
	iwork2 = ((n2 - n1 + 1) % size);
	start = rank * iwork1 + n1 + min(rank, iwork2);
	end = start + iwork1 - 1;
	if(iwork2 > rank)
		end = end + 1;

	return end - start + 1;
}



int main(int argc, char *argv[])
{

	//linked list
	node*head = NULL;
	

	//values used to store values into nodes
	int num_insert = 0;
	int row_i = 0;
	int col_j = 0;
	

	//file read in operations
	int m_Row = 8;
	int m_Col = 6;
	int f_row = 0;
	int curr_Col_count = 0;
	const char delimiter[1] = " ";


	//variables for MPI, some values assign are temps
	//NOTE: May need to change some values
	int rank;
	int size;

	//Initialize MPI and get rank and size
	MPI_Init(NULL, NULL);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if(size == 1)
	{
		printf("This program must be run with a minimum of 2 processes (-n 2)\n");
		return -1;
	}
	

	//Keep track of the start time so we can calculate the runtime at the end.
	clock_t begin;

	//Process zero is responsible for reading in the matrix from the file.
	if(rank == 0)
	{
		begin = clock();

		//Open the file for reading
		FILE *fp = fopen("data", "r");

		//Read in the size of the matrix
		const int BUFFER = 100;
		char line[BUFFER];

		//Get number of rows out of 100 byte buffer from line one in the matrix
		m_Row = atoi(fgets(line, BUFFER, fp));
		
		//Get number of columns out of 100 byte buffer from line two in the matrix
		m_Col = atoi(fgets(line, BUFFER, fp));

		//get the number of rows for the matrix
		f_row = m_Col * m_Row;


		//READ FROM FILE
		for(int i = 0; i < f_row; i++)
		{
			//Read a line up to the newline character
			fgets(line, BUFFER, fp);
			char *p = strtok(line, delimiter);
			
			//TODO: Linked List
			while(p != NULL)
			{
				if(curr_Col_count == 0)
				{
					num_insert = atoi(p);
				}
				if(curr_Col_count == 1)
				{
					row_i = atoi(p);
				}
				if(curr_Col_count == 2)
				{
					col_j = atoi(p);
				}

				//increment the count to get to next col that has a value
				curr_Col_count++;


				//igbore the spaces in the input between the file reads
				p = strtok(NULL, " ");
			
			}

			//create a node and store values into it
			head = prepend(head,num_insert,row_i,col_j);

			//reset the counter
			curr_Col_count = 0;
		}

		//Close the file
		fclose(fp);

		//print the linked list
		print(head);
	}


	//-----------------------HERE: Main MPI Operation-----------------------

	//Broadcast the matrix dimensions to all processes.
	MPI_Bcast(&m_Row, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(&m_Col, 1, MPI_INT, 0, MPI_COMM_WORLD);

	//Determine the chunk size for each process
	//NOTE: Already calculated above when determining how many rows to iterate to read file
	//int loc_dim = row * col;
	int loc dim = f_row;

	if(rank != 0)
	{
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

	//--------------------------------------------------------------------



	if(head != NULL)
	{
		printf("\n------ Deleting Linked List------\n");
		dispose(head);
		printf("All done!\n");
	}
	else
	{
		printf("Nothing else to delete...");
	}


	return 0;

}

