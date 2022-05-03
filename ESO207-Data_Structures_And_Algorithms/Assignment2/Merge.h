#ifndef MERGE_H
#define MERGE_H
#include<bits/stdc++.h>
#include"Basic.h"
using namespace std;

//data structure to store height and minimum value of a given tree
class res{
public:
    int height;
    int min;
};

//function that calculates and return the height and min element of a given tree 
res* height(node* T)
{
    node* curr=T;
    int h=0;
    while(!(curr->leaf))
    {
        curr=curr->child1;
        ++h;
    }
        res* ans=new res;
        ans->height=h+1;
        ans->min=curr->data;
        return ans;
}

//r1->root of first tree
//r2->root of second tree
//h1 and h2- height of respective trees
//min1 and min2 - min element of respective trees
//Constraint: elements(r1)<elements(r2) for all elements in trees
//returns {n1,n2,m}
answer* sub_merge(node* r1,node* r2, int h1,int h2,int min1,int min2)
{
    //case when tree1 is smaller than tree2
    if(h1<h2){
    if(h2==h1+1)   //when the height is reached in tree2 where tree1 will be inserted
    {   
        if(r2->children==2) //if twoNode then tree1 will become first child of tree2
        {
            r2->children=3;
            r2->child3=r2->child2;
            r2->child2=r2->child1;
            r2->child1=r1;
            r2->indic2=r2->indic1;
            r2->indic1=min2;
            return make_answer(r2,NULL,0);     // n1=threeNode,n2=NULL
        }
        else    //if threeNode then split into 2 twoNodes and fill the appropriate values
        {
            r2->children=2;  //firstt twoNode
            int num=r2->indic1;
            r2->indic1=r2->indic2;
            node* second=two_node(r1,r2->child1,min2);//second twoNode made after splitting 
            r2->child1=r2->child2;
            r2->child2=r2->child3;
            
            return make_answer(second,r2,num);
        }
    }
    else    //if we have not reached the desired height
    {
            if(r2->children==2)  //if twoNode recursively call sub_merge of firstchild of tree2 as we are dealing with the case of h2>h1
            {   
                answer* trial=sub_merge(r1,r2->child1,h1,h2-1,min1,min2);  //h2 decreases by 1 on recursive call as we move one level down
                if(trial->n2==NULL) {    //if n2==NULL r2 child1 will be updated by n1 and it will remain twoNode
                    r2->child1=trial->n1;
                    return make_answer(r2,NULL,0);
                }
                //if n2!=NULL then twoNode -> threeNode 
                r2->children=3;
                r2->child3=r2->child2;
                r2->child2=trial->n2;
                r2->child1=trial->n1;
                r2->indic2=r2->indic1;
                r2->indic1=trial->m;
                return make_answer(r2,NULL,0);
            }
            else //if threeNode
            {   
                answer* trial=sub_merge(r1,r2->child1,h1,h2-1,min1,min2);
                if(trial->n2==NULL) { //if n2==NULL r2 child1 will be updated by n1 and it will remain threeNode
                    r2->child1=trial->n1;
                    return make_answer(r2,NULL,0);
                }
                //if n2!=NULL then the threeNode will be splitted into 2 twoNodes
                r2->children=2;
                int num=r2->indic1;
                r2->indic1=r2->indic2;
                node* second=two_node(trial->n1,trial->n2,trial->m);
                r2->child1=r2->child2;
                r2->child2=r2->child3;
                return make_answer(second,r2,num);
            }    
    }
    }
    else  //another similar case when h1>h2(only change is now we need to traverse on the right side of tree1 to insert tree2 )
    {
       if(h1==h2+1)
       {
            if(r1->children==2)
            {
                r1->children=3;
                r1->child3=r2;
                r1->indic2=min2;
                return make_answer(r1,NULL,0);
            }
            else
            {
                r1->children=2;
                int num=r1->indic2;
                node* second=two_node(r1->child3,r2,min2);
                return make_answer(r1,second,num);
            }
       }
       else
       {
        
            if(r1->children==2)
            {   
                answer* trial=sub_merge(r1->child2,r2,h1-1,h2,min1,min2);
                if(trial->n2==NULL){
                    r1->child2=trial->n1;
                    return make_answer(r1,NULL,0);
                }

                r1->children=3;
                r1->child2=trial->n1;
                r1->child3=trial->n2;
                r1->indic2=trial->m;
                return make_answer(r1,NULL,0);
            }
            else
            {   
                answer* trial=sub_merge(r1->child3,r2,h1-1,h2,min1,min2);
                if(trial->n2==NULL){
                    r1->child3=trial->n1;
                    return make_answer(r1,NULL,0);
                }
                r1->children=2;
                int num=r1->indic2;
                node* second=two_node(trial->n1,trial->n2,trial->m);
                return make_answer(r1,second,num);
            }       
       }
    }
}

//Main merge function
//arguments: two trees T1 and T2 which needs to be merged
//T1 and T2 points to tthe root of the tree
//Constraint: elements(T1)< elements(T2) (true for all elements of T1 and T2)
//returns pointer to the root of the new merged tree
node* Merge(node* T1,node* T2)
{   //base case (if any one or both trees are NULL)
    if(T1==NULL) return T2;
    if(T2==NULL) return T1;

    //calculation of height and min element of both trees
    res* sub1=height(T1);
    res* sub2=height(T2);
    int h1=sub1->height;
    int min1=sub1->min;
    int h2=sub2->height;
    int min2=sub2->min;

    //when height is equal then a parent of T1 and T2(which is twoNode will be made and it will be the root of new tree formed)
    if(h1==h2)
        return two_node(T1,T2,min2);

    //sub_merge is a auxiliary function which will be called recursively
    answer* result=sub_merge(T1,T2,h1,h2,min1,min2);

    //if n2==NULL then n1 will be the root of the merged Tree
    if(result->n2==NULL) return result->n1;
    //else twoNode whose children are n1 and n2 will be the root of the merged tree
    else return two_node(result->n1,result->n2,result->m);
}

#endif