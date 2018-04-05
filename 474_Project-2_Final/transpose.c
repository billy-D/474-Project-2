/*--------------------------------------------------------------------
Author: Billy Dang, Sean McKean, Hassan Hamod
Program: Project 2
Date: 12/21/2017
File: transpose.c
Description: Sparse Matrix, Matrix Tanspose


Sources:
Linked List - http://www.zentut.com/c-tutorial/c-linked-list/

Docker MPI-Image
docker run --rm -it -v $(pwd):/project nlknguyen/alpine-mpich

-------------------------------------------------------------------*/
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


//Struct will help us create the linked list
typedef struct node
{
    int i, j, value;
    struct node *next;
} node;



//Struct designed to be used with MPI
//sending data to other processes
typedef struct nodeArr
{
    int i_Val, j_Val, v_Val;

}nodeArr;

//reference function so that we don't have to define the struct node everytime
//we create a new pointer
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
//creating the list itself
node* prepend(node* head,int data, int i, int j)
{
    node* new_node = create(data,head,i,j);
    head = new_node;
    return head;
}


//Print the linked list
void print(node*head)
{
    printf("\n-----------Values Read in from file to linked list------------------\n");
    node* reader = head;
    while(reader != NULL)
    {
        printf("Value: %d ", reader->value);
        printf("row: %d ", reader->i);
        printf("col: %d ", reader->j);

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
    node*reader = NULL;


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
    //control variable for storing the value into the nodeArr
    int index = 0;



    //finding max of i's and j's + store values into original matrix
    int temp_i, temp_j, max_Row, max_Col, temp_Val;
    int index2 = 0;
    int *i_Array = NULL;
    int *j_Array = NULL;

    //Matrices
    int *matrix = NULL;
    int *transpose = NULL;


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

        //Read in the size of the matrix
        const int BUFFER = 100;
        char line[BUFFER];
        

        //get number of rows
        m_Row = atoi(fgets(line,BUFFER,fp));

        //Get number of columns 
        m_Col = atoi(fgets(line, BUFFER, fp));
  
        //READ FROM FILE, the data for the linked list
        for(int i = 0; i < m_Row; i++)
        {
            //Read a line up to the newline character
            fgets(line, BUFFER, fp);
            char *p = strtok(line, delimiter);

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
        print(head);


        //pointer to head, assist in reading values from linked list into our designated arrays
        reader = head;

        //variables for finding dimension of matrix
        i_Array = malloc(num_Nodes*sizeof(int));
        j_Array = malloc(num_Nodes*sizeof(int));

        //need to put the head into struct
        while (reader != NULL )
        {
            //reader->value;
            temp_i = reader->i;
            temp_j = reader->j;

            //check if index2 does not surpass the buffer size by using num_Nodes
            if(index2 != num_Nodes)
            {
                i_Array[index2] = temp_i;
                j_Array[index2] = temp_j;
            }

            index2++;
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


        //Create the origianl matrix and allocate space, intitalize and set values to 0
        matrix = (int*) malloc(max_Row * max_Col * sizeof(int));
        for(int  i = 0; i < max_Row; i++)
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

        //again go through the list, and put values into corresponding location in matrix
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


        //print the original matrix
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

        printf("\n-----------Outputting Values Sent to Processes-----------\n\n");


    }

    
    //-----------------------HERE: Main MPI Operation-----------------------

    //specify MPI structure, since we aren't using just a single type of data
    //dealing with multiple types of data, at the same time we want our value 
    //to correspond with its location. Hence the use of struct since MPI can
    //deal with it
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

    //pointer to head, used to read and store values in NodeArr struct
    reader = head;

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


    //Needed for MPI scatterv and gatherv
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

    //Send parts of data to processes
    MPI_Scatterv(send_Buff, sendcount, displs, n_NodeObj, rec_Buff, 100, n_NodeObj, 0, MPI_COMM_WORLD);

    //perform i, j transpose operations
	for(int i = 0; i < sendcount[rank]; i++ )
	{
		int temp = rec_Buff[i].i_Val;
		rec_Buff[i].i_Val = rec_Buff[i].j_Val;
		rec_Buff[i].j_Val = temp;
	}

   
    // print what values were delivered to each process after transpose
    printf("\nProcess %d: ", rank);
    for (int i = 0; i < sendcount[rank]; i++) 
    {
    
     	printf("Value: %d\t", rec_Buff[i].v_Val);
    
    }
    printf("\n");

    
    //NOTE: Gatherv works if recvcout = 2, if 1, 2 of the values are butchered
    //Solution, recvcount should be the value of the number of processes given
    //Or else the values would get butchered
    //Collect the data into another buffer, for us to use (final_Buff)
    MPI_Gatherv(rec_Buff, size, n_NodeObj, final_Buff, sendcount, displs, n_NodeObj, 0, MPI_COMM_WORLD);


   

    if(0 == rank)
    {
        printf("\n------------------Gatherv operation complete-----------------------\n");
        //Create the transpose matrix and allocate space, intitalize and set values to 0
           
        for (int i = 0; i < num_Nodes; i++)
        {
            printf("Value: %d\t", final_Buff[i].v_Val);
            printf("row: %d\t", final_Buff[i].i_Val);
            printf("col: %d\t", final_Buff[i].j_Val);
            printf("\n");
        }

        //allocate space for the matrix transpose
        transpose = (int*) malloc(max_Row * max_Col * sizeof(int));
        for(int  i = 0; i < max_Row; i++)
        {
            for(int j = 0; j < max_Col; j++)
            {   
                *(transpose + i*max_Col + j) = 0;   
            }
        }


        //reuse this variables for matrix transpose
        temp_Val = 0;
        temp_i = 0;
        temp_j = 0;

        //place values into transpose matrix
        for (int i = 0; i < num_Nodes; i++)
        {

            //grab the values from the buffer
            temp_Val = final_Buff[i].v_Val;
            temp_i = final_Buff[i].i_Val;
            temp_j = final_Buff[i].j_Val;

            //now place them in the matrix
            for(int  i = 0; i < max_Row; i++)
            {
                for(int j = 0; j < max_Col; j++)
                {   
                     if(temp_i == i && temp_j == j)
                    {
                        *(transpose + i*max_Col + j) = temp_Val;
                    }
                }
            }
        }

        printf("\nMatrix Transpose\n\n");
        for(int i = 0; i < max_Row; i++)
        {
            for(int j = 0; j < max_Col; j++)
            {
                printf("%d  ",  *(transpose + i*max_Col + j));
                if (j == max_Col - 1)
                {
                    printf("\n\n");
                }
            }

        }


       

    }



    MPI_Finalize();


    //deallocate memory
    if(head != NULL)
    {
        printf("\n-----------------------Deallocating Memory--------------------------------\n");
        dispose(head);
        free(sendcount);
        free(displs);
        free(i_Array); 
        free(j_Array);
        free(matrix);
        free(transpose); 
        printf("All done!\n");
    }

    return 0;

}

