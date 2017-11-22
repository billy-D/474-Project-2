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
void deleteList(Node*&);


int main(){
    //declare a pointer to use for list
	NodeType * head = nullptr;

	//used to pass a value into to add to list
    int value = 0;

    //menu choice
	char choice = ' ';

	//used to store the return value from find, assign it to be false by default
    bool returnValue = false;
    
    do 
	{
		//list options
		listOptions();

		//get users choice based on menu
		cin >> choice;

		//convert lowercase to uppercase if any lowercase inputs
		choice = toupper(choice);

		//spaceing between menu and user interactions
		cout << endl;

		switch (choice) {
		case 'A':
			//insert a value at the top of the list as the head
			cout << "Enter a value to insert: ";
			cin >> value;

			//call the insert function
			insert(head, value);


			break;
		case 'P':
			//check if the list is empty
			if (head == nullptr)
			{
				cout << "List is empty at the moment...\n";
			}
			else
			{
				//print the linked list by calling the printList function
				cout << "List of Nodes: Newest -> Oldest\n";
				printList(head);
			}
			system("PAUSE");
			system("CLS");
			break;

		case 'Q':
			//quit
			
			if (head == nullptr)
			{
				cout << "List is empty, good to go!\n";
			}
			else
			{
				//print an error message
				cout << "List still exist, but no worries calling deleteList function... " << endl;

				//call deleteList again to kill list
				deleteList(head);
			}
			
			system("PAUSE");
			system("CLS");
			break;

		default:
			//error message if user enters something different
			cout << "Invalid option!\n";
			system("PAUSE");
			system("CLS");
			break;
		}


	} while (choice != 'Q');

    cout << endl;
    return 0;
}


/*-------------------------------------------------------
function: listOptions
parameters: none
purpose: provide user with a list of choices to use
-------------------------------------------------------*/
void listOptions()
{
	cout << "=============Menu=================\n";
	cout << "A -> Add\n";
	cout << "P -> Print\n";
	cout << "Q -> Quit\n";
	cout << "===================================\n";

	cout << "Enter a valid option: ";

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


/*-------------------------------------------------------
function: deleteList
parameters: A pointer that is a passed by reference 
purpose: deleting every node in the linked list
-------------------------------------------------------*/
void deleteList(Node*&head)
{

	//Need a loop to iterate through each Node in the list
	while (head != nullptr)
	{

		//Need a pointer to use to delete Node
		Node*Temp = head;

		//At the top of the list already, need to move the head pointer to the next node
		head = head->nextPtr;

		//Temp is still linked to the next node, cut its link to the rest of the list
		Temp->nextPtr = nullptr;
		delete Temp;

		Temp = nullptr;
	}

}