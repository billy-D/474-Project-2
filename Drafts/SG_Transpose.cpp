/*--------------------------------------------
Names: Billy Dang, Sean Mckean, Hassan Hamod
File: SG_Transpose.cpp
----------------------------------------------*/
#include <iostream>
#include "Node.h"
//#include <mpi.h>
using namespace std;


// Function Declarations
void printList(Node*);
void insert(Node*&, int, int, int);
void deleteList(Node*&);
void listOptions();


int main() {
	//declare a pointer to use for list
	Node * head = nullptr;

	//used to pass a value into to add to list
	int value = 0;

	//pass in the position of the elmement corresponding to sparse matrix
	int pos_i = 0;
	int pos_j = 0;

	//menu choice
	char choice = ' ';

	//used to store the return value from find, assign it to be false by default
	bool returnValue = false;

	// array to hold the coordinates, and determine max
	int max_i = 0;
	int max_j = 0;
	int array_i[30];
	int array_j[30];
	int index = 0;

	//Original matrix, with some arbitrary value that will change 
	int orig_Matrix[10][10]; 

	// For MPI operations
	int rank, result;

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

			cout << "Position corresponding to sparse matrix (i,j): ";
			cin >> pos_i;
			cin >> pos_j;

			//call the insert function
			insert(head, value, pos_i, pos_j);

			if (index < 30) {
				array_i[index] = pos_i;
				array_j[index] = pos_j;

				//increment index
				index++;
			}
			else {
				cout << "Error: array_i and array_j, reached max capacity\n";
			}

			break;

		case 'P':
			//check if the list is empty
			if (head == nullptr) {
				cout << "List is empty at the moment...\n";
			}
			else {
				//print the linked list by calling the printList function
				cout << "List of Nodes: Newest -> Oldest\n";
				printList(head);
			}
			break;
		case 'Q':
			cout << "Finishing setup, exiting interactive menu...\n";
			max_i = array_i[0];
			max_j = array_j[0];

			//find max
			for (int a = 0; a < index; a++) {
				if (max_i < array_i[a]) {
					max_i = array_i[a];
				}
				if (max_j < array_j[a]) {
					max_j = array_j[a];
				}
			}

			//cout << "Matrix Dimensions: " << max_i << " x " << max_j << "\n";
			system("PAUSE");

		default:
			//error message if user enters something different
			cout << "Invalid option!\n";
			break;
		}


	} while (choice != 'Q');
	cout << endl;


	// create the matrix with new dimensions
	orig_Matrix[max_i][max_j];

	//----------------Begin MPI operations here----------------------------------













	//---------------------------------------------------------------------------



	//Once we are done, deallocate used memory before exiting
	deleteList(head);
	system("PAUSE");
	return 0;
}


/*-------------------------------------------------------
function: listOptions
parameters: none
purpose: provide user with a list of choices to use
-------------------------------------------------------*/
void listOptions() {
	cout << "=============Menu=================\n";
	cout << "A -> Add\n";
	cout << "P -> Print\n";
	cout << "Q -> Quit Setup + Begin MPI OP's\n";
	cout << "===================================\n";

	cout << "Enter a valid option: ";

}


/*-------------------------------------------------------
function: insert
parameters: A pointer passed by reference and integer value
purpose: Inserting values as the head of the linked list
-------------------------------------------------------*/
void insert(Node*&head, int value, int pos_i, int pos_j) {

	//check if the value is non-zero
	if (value > 0) {

		//declare a new pointer and assign it to head
		Node*currentPtr = head;

		//make a new node 
		currentPtr = new Node;

		//give the new node a value
		currentPtr->info = value;

		//give the new node the position corresponding to matrix
		currentPtr->position_i = pos_i;
		currentPtr->position_j = pos_j;

		//set the new node nextptr to be the head to link the new node with previous node
		currentPtr->nextPtr = head;

		//set the new node to be the new head
		head = currentPtr;

		cout << value << " is added to the list!\n";
	}
	else {
		cout << "Insert: NON-ZERO elements!\n";
	}
}


/*-------------------------------------------------------
function: printList
parameters: A pointer
purpose: Printing all the values in the linked list
-------------------------------------------------------*/
void printList(Node*head) {

	//declared a new pointer and assign it to head
	Node*currentPtr = head;

	//while we haven't reached the end of the list
	while (currentPtr != nullptr) {
		//print the content of the linked list
		cout << currentPtr->info << ":( ";
		cout << currentPtr->position_i << ",";
		cout << currentPtr->position_j << ") ";

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
void deleteList(Node*&head) {

	if (head == nullptr) {
		cout << "List is empty, no need to worry!\n";
	}
	else {
		cout << "ERROR: List is not empty, PANIC!\n";

		//Need a loop to iterate through each Node in the list
		while (head != nullptr) {

			//Need a pointer to use to delete Node
			Node*Temp = head;

			//At the top of the list already, need to move the head pointer to the next node
			head = head->nextPtr;

			//Temp is still linked to the next node, cut its link to the rest of the list
			Temp->nextPtr = nullptr;
			delete Temp;

			// set temp to be null again for the next round of node deletions
			Temp = nullptr;
		}

		cout << "JK, its all good!\n";

	}

}