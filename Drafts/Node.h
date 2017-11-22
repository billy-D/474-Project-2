/*--------------------------------------------
Names: Billy Dang, Sean Mckean, Hassan Hamod
File: Node.h
----------------------------------------------*/
#include <iostream>
using namespace std;


// Class will help us make nodes for our linked list
class Node(){
    Node(int = 0);
    Node * nextPtr;
    int info;
    int position;
};

// Define the constructor
Node::Node(int temp_data, int temp_pos){
    info = temp_data;
    position = temp_pos;
    nextPtr = 0;
}

