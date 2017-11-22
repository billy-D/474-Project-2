/*--------------------------------------------
Names: Billy Dang, Sean Mckean, Hassan Hamod
File: SG_Transpose.cpp
----------------------------------------------*/

#include <iostream>
#include "Node.h"
#include <mpi.h>


// Function Declarations
void printList(Node*);
void insert(Node*&, int);


int main(){

    return 0;
}



/*-------------------------------------------------------
function: insert
parameters: A pointer passed by reference and integer value 
purpose: Inserting values as the head of the linked list
-------------------------------------------------------*/
void insert(Node*&head, int value)
{
	//declare a new pointer and assign it to head
	Node*currentPtr = head;

	//make a new node 
	currentPtr = new Node;

	//give the new node a value
	currentPtr->info = value;

	//set the new node nextptr to be the head to link the new node with previous node
	currentPtr->nextPtr = head;

	//set the new node to be the new head
	head = currentPtr;

	cout << value << " is added to the list!\n";
	system("PAUSE");
	system("CLS");
}


/*-------------------------------------------------------
function: printList
parameters: A pointer
purpose: Printing all the values in the linked list
-------------------------------------------------------*/
void printList(Node*head)
{
	//declared a new pointer and assign it to head
	Node*currentPtr = head;

	//while we haven't reached the end of the list
	while (currentPtr != nullptr)
	{
		//print the content of the linked list
		cout << currentPtr->info << " ";

		//move onto the next node
		currentPtr = currentPtr->nextPtr;
	}

	//spacing
	cout << endl;

}
