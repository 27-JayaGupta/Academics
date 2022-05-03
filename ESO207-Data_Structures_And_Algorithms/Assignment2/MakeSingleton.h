#include <iostream>
#include"Basic.h"
#include "Merge.h"
#include "Extract.h"
using namespace std;

node* MakeSingleton(int num)
{
    return make_node(true,0,0,0,NULL,NULL,NULL,num);
}

//Test function to test the program 
void Test(){
	node *T = MakeSingleton(1);
    for(int i = 2 ;i<= 500;i++){
    	T= Merge(T,MakeSingleton(i));
    }

	node *U = MakeSingleton(777);
	for(int i = 778 ;i<=1000;i++)
	U= Merge(U,MakeSingleton(i));

	node *V=Merge(T,U);
	Extract(V);
}