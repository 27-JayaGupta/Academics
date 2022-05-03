#include<iostream>
#include <bits/stdc++.h>
#include<vector>
#include<cmath>
using namespace std;
typedef long long ll;

class node{
public:
    ll exponent;
    ll coefficient;
    class node* next;
    class node* prev; 
};
void printlist(node* dll)
{

    node* ptr=dll->next;
    while(ptr!=dll)
    {
        cout<<ptr->coefficient<<" "<<ptr->exponent<<" ";
        ptr=ptr->next;
    }
}

// Function to find the pointer to the central element n a list. 
// This is done using two pointers one which moves twice as fast
// as the other. When the faster pointer reaches the end of the
// list,the slower pointer points to the central element.
node* middle_ptr(node* list)
{
    node* slow=list->next;
    node* fast=list->next->next;
    while((fast!=list)&&(fast->next!=list))
    {
        slow=slow->next;
        fast=fast->next->next;
    }
    return slow;
}
node* make_sentinel()
{
    node* sentinel=new node;
    sentinel->exponent=-1;
    sentinel->coefficient=0;
    sentinel->next=sentinel;
    sentinel->prev=sentinel;
    return sentinel;
}
node* make_node(ll exponent,ll coefficient)
{
    node* new_node=new node;
    new_node->exponent=exponent;
    new_node->coefficient=coefficient;
    new_node->next=NULL;
    new_node->prev=NULL;
    return new_node;
}

// Function to append an element to the existing list. 
node* append(node* dll,ll exponent,ll coefficient)
{
    node* new_node=make_node(exponent,coefficient);
    if (coefficient==0)    return dll;
    // The new element goes to the end of the list. Also, 
    // no node is added if the coefficint argument is zero.
    
    node* tail=dll->prev;
    if(tail->exponent==exponent)
    {
        tail->coefficient=tail->coefficient+coefficient;
        return dll;
    }  
    // If the value of the exponent is equal to the last node,
    // then the coefficent is added to that node.

    new_node->next=tail->next;
    new_node->prev=tail;
    tail->next->prev=new_node;
    tail->next=new_node;
    return dll;
}

// This function works in exactly the same manner as the addtion fucntion
node* double_merge(node* p,node* q)
{
    node* sum=make_sentinel();
    node* ptr1=p->next;
    node* ptr2=q->next;
    while((ptr1!=p) &&(ptr2!=q))
    {
        if(ptr1->exponent>ptr2->exponent)
        {
            sum=append(sum,ptr2->exponent,ptr2->coefficient);
            ptr2=ptr2->next;
        }
        else if(ptr1->exponent<ptr2->exponent)
        {
            sum=append(sum,ptr1->exponent,ptr1->coefficient);
            ptr1=ptr1->next;
        }
        else
        {
            if(ptr1->coefficient+ptr2->coefficient!=0)
                sum=append(sum,ptr1->exponent,ptr1->coefficient+ptr2->coefficient);
            ptr1=ptr1->next;
            ptr2=ptr2->next;
        }
    }
    if (ptr1==p)
    {
        while(ptr2!=q)
        {
            sum=append(sum,ptr2->exponent,ptr2->coefficient);
            ptr2=ptr2->next;    
        }
    }
    if (ptr2==q)
    {
        while(ptr1!=p)
        {
            sum=append(sum,ptr1->exponent,ptr1->coefficient);
            ptr1=ptr1->next;    
        }
    }
    return sum;
}
node* merge_sort(node* arr)
{
    if(arr->next->next==arr)  return arr;//If the list contains a single element then the list is already sorted.
    node* center_pointer=middle_ptr(arr);//Now we split the list at the central element and make two linked lists both 
                                         //with sentinel nodes.
    node* second_list=make_sentinel();
    node* tail=arr->prev;
    node* second_head=center_pointer->next;
    arr->prev=center_pointer;
    center_pointer->next=arr;
    second_list->next=second_head;
    second_head->prev=second_list;
    tail->next=second_list;
    second_list->prev=tail;
    return double_merge(merge_sort(arr),merge_sort(second_list));   
    //This is the recursive step of merge sort, if thw two lists created are
    //then the final list is obtained by merging them into one sorted list.  
}


//Loops over the two polynomials and returns an unorted list containing the polynomial terms.
node* multiply(node* p,node* q)
{
    node* ptr1=p->next;
    node* ptr2=q->next;
    node* arr=make_sentinel();
    while(q!=ptr2)
    {
 
        while(ptr1!=p)
        {
            arr=append(arr,ptr2->exponent+ptr1->exponent,(ptr1->coefficient)*(ptr2->coefficient));
            ptr1=ptr1->next;
        }
    ptr1=ptr1->next;
    ptr2=ptr2->next;
    }
    return merge_sort(arr);
}
node *input(ll n){

    node *sentinel=make_sentinel();

    for(ll i=0;i<n;i++){
        ll c;
        ll e;
        cin>>c>>e;

        sentinel=append(sentinel,e,c);

    }

    return sentinel;
}
int main() {
    ll n,m;
    cin>>n>>m;
    node* p=input(n);
    node* q=input(m);
    node* product=multiply(p,q);
    printlist(product);
    return 0;
}