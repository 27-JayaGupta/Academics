#include<iostream>
using namespace std;

//Print all the elements stored in tree T1 in ascending order
void Extract(node* T1) {
    if (T1->leaf) {
        cout << T1->data << ",";
        return;
    }

    if (T1->children==2) {

        Extract(T1->child1);
        Extract(T1->child2);
    }

    if (T1->children==3) {

        Extract(T1->child1);
        Extract(T1->child2);
        Extract(T1->child3);
    }
}