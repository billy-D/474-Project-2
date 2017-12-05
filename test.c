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


typedef struct node
{
	int i, j, value;
	struct node *next;
} node;


typedef struct nodeArr
{
	int i_Val, j_Val, v_Val;
} nodeArr;


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
	int num_Nodes = 0;


	//variables for MPI, some values assign are temps
	//NOTE: May need to change some values
	int rank;
	int size = argv[1];

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


			//increment the number of nodes created, and added to the linked list
			num_Nodes++;


			//reset the counter
			curr_Col_count = 0;
		}

		//Close the file
		fclose(fp);

		//print the linked list
		print(head);
	

	}//0 ends
		//-----------------------HERE: Main MPI Operation-----------------------


		//specify MPI structure
		int blocks[3] = {1,1,1};
		MPI_Datatype types[3] = {MPI_INT, MPI_INT, MPI_INT};
		MPI_Aint displacements[3];

		MPI_Datatype n_NodeObj;
		MPI_Aint intex;

		MPI_Type_extent(MPI_INT, &intex);
		displacements[0] = 0;
		displacements[1] = intex;
		displacements[2] = intex + intex;

		MPI_Type_struct(3, blocks, displacements, types, &n_NodeObj);
		MPI_Type_commit(n_NodeObj);


		//create an array that will hold values in linked for other processes
		nodeArr holder[] = malloc(num_Nodes*sizeof(nodeArr));
		nodeArr rec_buff[] = malloc(num_Nodes*sizeof(nodeArr));



		//control variable for storing the value into the nodeArr
		int index = 0;

		//pointer to head, used to read and store values in NodeArr struct
		node*reader = head;

		//need to put the head into struct
		while (reader != NULL)
		{
			holder[index].v_Val = reader->value;
			holder[index].i_Val = reader->i;
			holder[index].j_Val = reader->j;

			index++;
			reader = reader->next;

		}

		int rem = num_Nodes % size;
		int sendcount[] = malloc(sizeof(int)*size);
		int displs[] = malloc(sizeof(int)*size);
		
		for (int i =0; i<size;i++)
		{
			sendcounts[i] = num_Nodes/size;
			if(rem > 0)
			{
				sendcounts[i]++;
				rem--;
			}
		}

		MPI_Scatterv(&holder,sendcounts,displs, nodeArr, &rec_buff, num_Nodes, nodeArr, 0, MPI_COMM_WORLD);

	for(int i =0; i<sendcounts[rank];i++ )
	{
		int temp = rec_buff[i].i_Val;
		rec_buff[i].i_Val = rec_buff[i].j_Val;
		rec_buff[i].j_Val = temp;
	}
	
	if (rank == 0)
	{
		nodeArr transpose[] = malloc(num_Nodes*sizeof(nodeArr));
	}

	MPI_Gather(&transpose, 1, nodeArr, transpose, 1, nodeArr, 0, MPI_COMM_WORLD);

	if(rank == 0)
	{
		int count =0;
		//output matrix
		for (int i = 0; i < m_Col; i++)
		{
			printf("/");
			for(int j = 0; j < m_Row; j++)
			{
				if(transpose[count].i_Val == i &&transpose[count].jVal == j)
				{
					printf("%d ", transpose[count].v_Val);
					count++;
				}
				else
				{
					printf("0 ", );
				}

			}
		}
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

