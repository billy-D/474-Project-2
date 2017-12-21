/*--------------------------------------------------------------------
Author: Billy Dang, Sean McKean, Hassan Hamod
Program: Project 2
Date: 12/02/2017
File: transpose.c
Description: Sparse Matrix, Matrix Tanspose


Sources:

Linked List - http://www.zentut.com/c-tutorial/c-linked-list/
2D Dyanmic Arrays in C - http://www.geeksforgeeks.org/dynamically-allocate-2d-array-c/

Docker MPI-Image
docker run --rm -it -v $(pwd):/project nlknguyen/alpine-mpich

------------------=-------------------------------------------------*/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>



typedef struct node
{
    int i, j, value;
    struct node *next;
} node;


typedef void (*callback_1)(node* value);



//helper function to help set up the nodes
node* create(int data, node* next, int row, int col)
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

    //set ptr to null
    reader = NULL;
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
    node*reader = head;

    //variables used to help store values into nodes
    int num_insert = 0;
    int row_i = 0;
    int col_j = 0;


    //file read in operations
    int m_Row = 8;
    int m_Col = 6;
    int curr_Col_count = 0;
    const char delimiter[1] = " ";
    int num_Nodes = 0;


    //finding max of i's and j's + store values into original matrix
    int temp_i, temp_j, max_Row, max_Col, temp_Val;
    int index = 0;



    //variables for MPI, some values assign are temps
    //NOTE: May need to change some values
    int send[4], recv[3];
    int rank;
    int size;
    int k;


    //Initialize MPI and get rank and size
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    //revise later
    if(size == 1)
    {
        printf("This program must be run with a minimum of 2 processes (-n 2)\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
        return -1;
    }


    //Process zero is responsible for reading in the matrix from the file.
    if(rank == 0)
    {

        //Use instance to, Open the file for reading
        FILE *fp = fopen("data.txt", "r");

        //check if the file exist
        if(fp == NULL) 
        {
            printf("Cannot open file \n") ;
            exit(0);
        }

        const int BUFFER = 100; 
        char line[BUFFER];
        
        //get number of rows
        m_Row = atoi(fgets(line,BUFFER,fp));

        //Get number of columns 
        m_Col = atoi(fgets(line, BUFFER, fp));

        //TEST: print number of rows and cols
        printf("Number of Rows %d\n", m_Row);
        printf("Num of Col %d\n", m_Col);

   
        //READ FROM FILE
        for(int i = 0; i < m_Row; i++)
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


        //print the number of nodes created
        printf("Number of Nodes Created %d\n", num_Nodes);

        //print the linked list
        print(head);


        //pointer to head, assist in reading values from linked list into our designated arrays
        reader = head;


        //variables for finding dimension of matrix
        int *i_Array = malloc(num_Nodes*sizeof(int));
        int *j_Array = malloc(num_Nodes*sizeof(int));

        //need to put the head into struct
        while (reader != NULL )
        {
            //reader->value;
            temp_i = reader->i;
            temp_j = reader->j;

            if(index != num_Nodes)
            {
                i_Array[index] = temp_i;
                j_Array[index] = temp_j;
            }

            index++;
            reader = reader->next;

        }
        //set to null to dereference pointer
        reader = NULL;

        //Need to determine max of i and j for size of matrix
        max_Row = i_Array[0];
        max_Col = j_Array[0];

        for(int i = 0; i < num_Nodes; i++)
        {
            if(max_Row < i_Array[i])
            {
                max_Row = i_Array[i];
            }

             if(max_Col < j_Array[i])
            {
                max_Col = j_Array[i];
            }
            
        }

        //add 1 to get the actual size dimension of matrix
        max_Row += 1;
        max_Col += 1;

        //print the dimension of i and j
        printf("Max Row %d\n", max_Row);
        printf("Max Col %d\n", max_Col);

       
    
    } //end of process 0

    //-----------------------HERE: Main MPI Operation-----------------------

     //Create the origianl matrix and allocate space, intitalize and set values to 0
        int *matrix = (int*) malloc(max_Row * max_Col * sizeof(int));
        for(int i = 0; i < max_Row; i++)
            {
                for(int j = 0; j < max_Col; j++)
                {   
                    *(matrix + i*max_Col + j) = 0;   
                }
            }

        
        //reuse some of the old variables to help store values into original matrix
        temp_Val = 0;
        temp_i = 0;
        temp_j = 0;

        //Note: Set reader back to head
        reader = head;

        while (reader != NULL)
        {
            temp_Val = reader->value;
            temp_i = reader->i;
            temp_j = reader->j;


            for(int i = 0; i < max_Row; i++)
            {
                for(int j = 0; j < max_Col; j++)
                {
                    //if coordinates match, then place value into corresponding spot
                    //in matrix
                    if(temp_i == i && temp_j == j)
                    {
                        *(matrix + i*max_Col + j) = temp_Val;
                    }
                }
            }
            
            //Traverse to the next node in linked list
            reader = reader->next;

        }

        //Again dereference reader 
        reader = NULL;


        printf("\nOriginal Matrix\n\n");
        for(int i = 0; i < max_Row; i++)
        {
            for(int j = 0; j < max_Col; j++)
            {
                printf("%d  ",  *(matrix + i*max_Col + j));
                if (j == max_Col - 1)
                {
                    printf("\n\n");
                }
            }

        }

   



    MPI_Finalize();

   //--------------------------------------------------------------------

    //deallocate memory
    if(head != NULL)
    {
        printf("\n------ Deallocating Memory------\n");
        dispose(head);
       
        //free(sendcount);
        //free(displs);
        printf("All done!\n");
    }

    return 0;

}



/*------------------------Output---------------------------
/project/Documents/GitHub/474-Project-2 $ mpirun -n 2 ./test
Number of Rows 4
Num of Col 3
Number of Nodes Created 4
11 1 1
45 1 0
23 0 1
22 0 0
Max Row 2
Max Col 2

Original Matrix

22  23

45  11


------ Deallocating Memory------
All done!
/project/Documents/GitHub/474-Project-2 $




-----------------------------------------------------------*/