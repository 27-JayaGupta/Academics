#include <iostream>
#include<stdlib.h>
using namespace std;

typedef long long ll;

typedef struct node{
    ll coeff;
    ll exp;
    node *next;
    node *prev;
}node;

//creates a new node
node *make_node(ll c,ll e){
    node *temp=new node();
    temp->coeff=c;
    temp->exp=e;
    temp->next=NULL;
    temp->prev=NULL;
    return temp;
}

//used for taking input [p(x) and q(x) pol] 
node *input(ll n){

    node *sentinel=make_node(0,0);
    sentinel->next=sentinel;
    sentinel->prev=sentinel;

    for(ll i=0;i<n;i++){
        ll c;
        ll e;
        cin>>c>>e;

        node *temp=make_node(c,e);
        temp->prev=sentinel->prev;
        temp->next=sentinel;
        sentinel->prev->next=temp;
        sentinel->prev=temp;
    }

    return sentinel;
}

//prints list
void print(node *head){

    if(head->next!=head){

        node *curr=head->next;
        while(curr!=head){
            
            cout<<curr->coeff<<" "<<curr->exp<<" ";
            curr=curr->next;
        }
        cout<<"\n";
    }
}

void append(ll c,ll e, node *sentinel){   //appends node in the final ans list
    node *temp=make_node(c,e);
    temp->prev=sentinel->prev;
    temp->next=sentinel;
    sentinel->prev->next=temp;
    sentinel->prev=temp;
}

void addition(node *h1,node *h2){

    node *sentinel_ans=make_node(0,0);  //final list to store the final ans
    sentinel_ans->prev=sentinel_ans;
    sentinel_ans->next=sentinel_ans;

         node *c1=h1->next,*c2=h2->next;
         while(c1!=h1 && c2 !=h2){          //iterate until any one list finishes
             if(c1->exp < c2->exp)  {
                append(c1->coeff,c1->exp,sentinel_ans);    //append pushes a node in the ans list
                 c1=c1->next;
             }
             else if(c2->exp < c1->exp) {
                 append(c2->coeff,c2->exp,sentinel_ans);
                 c2=c2->next;
             }
             else if(c2->exp == c1->exp) {
                 if(c1->coeff+c2->coeff != 0) append(c1->coeff+c2->coeff,c1->exp,sentinel_ans);
                 c1=c1->next;
                 c2=c2->next;
             }
         }

         if(c1==h1){           //if c1 reaches the end of List add remaining element of c2 in the final ansList.
             while(c2!=h2){
                 append (c2->coeff,c2->exp,sentinel_ans);
                 c2=c2->next;
             }
         }
         else if(c2==h2){      // if c2 reaches the end of List add remaining element of c1 in the final ansList.
             while(c1!=h1){
                 append(c1->coeff,c1->exp,sentinel_ans);
                 c1=c1->next;
             }
         }
        
    print(sentinel_ans);        //print the final list
}

int main(){
    ll n,m;
    cin>>n>>m;

    node *head1,*head2;          //head always points towards sentinel node of the list
    head1=input(n);
    head2=input(m);
    addition(head1,head2);

    return 0;
}