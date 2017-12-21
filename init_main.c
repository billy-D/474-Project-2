/*--------------------------------------------------------------------
Author: Billy Dang, Sean McKean, Hassan Hamod
Program: Project 2
Date: 12/02/2017
File: transpose.c
Description: Sparse Matrix, Matrix Tanspose


Sources:

Linked List - http://www.zentut.com/c-tutorial/c-linked-list/

Docker MPI-Image
docker run --rm -it -v $(pwd):/project nlknguyen/alpine-mpich

------------------=-------------------------------------------------*/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
//#include <time.h>


typedef struct node
{
    int i, j, value;
    struct node *next;
} node;



//used with MPI
typedef struct nodeArr
{
    int i_Val, j_Val, v_Val;

}nodeArr;

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


    //variables used to help store values into nodes
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
    int size;


    //Initialize MPI and get rank and size
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if(size == 1)
    {
        printf("This program must be run with a minimum of 2 processes (-n 2)\n");
        return -1;
    }


    //Keep track of the start time so we can calculate the runtime at the end.
    //clock_t begin;

    //Process zero is responsible for reading in the matrix from the file.
    if(rank == 0)
    {
        //begin = clock();

        //Use instance to, Open the file for reading
        FILE *fp = fopen("data.txt", "r");

        //check if the file exist
        if(fp == NULL) 
        {
            printf("Cannot open file \n") ;
            exit(0);
        }

        //Read in the size of the matrix
        const int BUFFER = 100;
        char line[BUFFER];
        

        //get number of rows
        m_Row = atoi(fgets(line,BUFFER,fp));

        //Get number of columns 
        m_Col = atoi(fgets(line, BUFFER, fp));

        //TEST: print number of rows and cols
        printf("Num Row %d\n", m_Row);
        printf("Num Col %d\n", m_Col);

   
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

        //print the linked list
        print(head);


    }

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
    MPI_Type_commit(&n_NodeObj);


    //create an array that will hold values in linked for other processes
    //all processes made the same array
    nodeArr send_Buff[100] = {0};
   
    //array for the recv buffer
    nodeArr rec_Buff[100] = {0};

    //last destination when all ops are completed buffer to be sent to
    nodeArr final_Buff[100] = {0};

    //control variable for storing the value into the nodeArr
    int index = 0;

    //pointer to head, used to read and store values in NodeArr struct
    node*reader = head;

    //need to put the head into struct
    while (reader != NULL )
    {
        send_Buff[index].v_Val = reader->value;
        send_Buff[index].i_Val = reader->i;
        send_Buff[index].j_Val = reader->j;

        index++;
        reader = reader->next;

    }

    //set ptr to null
    reader = NULL;



    int rem = size % size;
    int*sendcount = malloc(sizeof(int)*size);
    int*displs = malloc(sizeof(int)*size);
    int sum = 0;


    //calculate "chunk size" send counts and displacements
    for (int i = 0; i < size; i++)
    {
        sendcount[i] = (size*size)/size;
        if(rem > 0)
        {
            sendcount[i]++;
            rem--;
        }

        displs[i] = sum;
        sum += sendcount[i];
    }

    //see how many value are being sent to each of the processes
    if (0 == rank)
    {
        for (int i = 0; i < size; i++)
        {
            printf("sendcount[%d] = %d\tdispls[%d] = %d\n", i, sendcount[i], i, displs[i]);
        }
    }
    
    //
    MPI_Scatterv(send_Buff, sendcount, displs, n_NodeObj, rec_Buff, 100, n_NodeObj, 0, MPI_COMM_WORLD);



    // print what values were delivered to each process 
    printf("%d: ", rank);
    for (int i = 0; i < sendcount[rank]; i++) 
    {
     	printf("%d\t", rec_Buff[i].v_Val);     
    
    }
    printf("\n");

    
   // MPI_Gatherv(rec_Buff, 2 , n_NodeObj, send_Buff, sendcount, displs, n_NodeObj, 0, MPI_COMM_WORLD);
    


    


    MPI_Finalize();

   //--------------------------------------------------------------------

    //deallocate memory
    if(head != NULL)
    {
        printf("\n------ Deallocating Memory------\n");
        dispose(head);
        free(sendcount);
        free(displs);
        printf("All done!\n");
    }

    return 0;

}

