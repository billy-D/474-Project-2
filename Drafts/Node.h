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
    int position_i;
    int position_j;
};

// Define the constructor
Node::Node(int temp_data, int temp_pos_i, int temp_pos_j){
    info = temp_data;
    position_i = temp_pos_i;
    position_j = temp_pos_j;
    nextPtr = 0;
}

