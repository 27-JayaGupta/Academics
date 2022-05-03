//stores basic definition of commonly used data types in program and their utility functions
#ifndef BASIC_H
#define BASIC_H
#include<iostream>
using namespace std;

//data structure to represent single node of 2-3 tree
class node{
    public:
        bool leaf;      //whether a node is leaf node or not
        int children;   //stores number of children of a given node
        int indic1;     //indic1 and indic2 are the values stored in two or three node
        int indic2;     
        class node* child1;  //stores pointer to respective child of given node 
        class node* child2;
        class node* child3;
        int data;           //if a node is leaf node than the data(or element) is stored in this field
};

// defines data type of return from merge function
//n1 and n2 are two-three trees
//m is the minimum value in n2 tree if n2 is not null
// n1 and n2 are the left and right trees obtained after merge function at certain level
class answer{
public:
    node* n1;
    node* n2;
    int m;
};

//utility function to make node of type answer defined above
answer* make_answer(node* n1, node* n2, int m)
{
    answer* ans=new answer();
    ans->n1=n1;
    ans->n2=n2;
    ans->m=m;
    return ans;
}

//utility function to make node of two-three tree
node* make_node(bool leaf, int children, int indic1, int indic2, node* c1,node* c2, node* c3, int data)
{
    node* ans=new node();
    ans->leaf=leaf;
    ans->children=children;
    ans->indic1=indic1;
    ans->indic2=indic2;
    ans->child1=c1;
    ans->child2=c2;
    ans->child3=c3;
    ans->data=data;
    return ans;
}

//make twoNode
node * two_node(node* n1,node* n2, int m)
{
    return make_node(false,2,m,0,n1,n2,NULL,0);
}
//make threeNode
node * three_node(node* n1, node* n2, node* n3,int indic1,int indic2)
{
    return make_node(false,3,indic1,indic2,n1,n2,n3,0);
}
//make leafNode
node* leaf(int num)
{
    return make_node(true,0,0,0,NULL,NULL,NULL,num);
}

#endif
