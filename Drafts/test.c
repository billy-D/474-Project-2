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
	int rank = 0;
	int size = 0;

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

		//get the number of rows fort the matrix
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




	//Main MPI Operation







	if(head != NULL)
	{
		printf("\n------ Deleting Linked List------");
		dispose(head);
		printf("\n");
	}
	else
	{
		printf("Nothing else to delete...");
	}


	return 0;

}

