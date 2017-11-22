/*--------------------------------------------
Names: Billy Dang, Sean Mckean, Hassan Hamod
File: Node.h
----------------------------------------------*/
#include <iostream>


// Class will help us make nodes for our linked list
class Node(){
    Node(int = 0);
    Node * nextPtr;
    int info;
};

// Define the constructor
Node::Node(int data){
    info = data;
    nextPtr = 0;
}

