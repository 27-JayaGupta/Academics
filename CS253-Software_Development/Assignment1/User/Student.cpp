#pragma once
#include <bits/stdc++.h>
#include <fstream>
#include <string>
#include <iostream>
#include "user.cpp"
using namespace std;

class Student:public User{
	public:
		vector<int> issuedBooks;
		int dues;

	public:
		Student(){
			cout<<"-------------------\n";
			cout<<"Enter help for information on more commands\n";
		}
		void help();
};

void Student::help(){

	cout<<"----------------------------\n";
	cout<<"Here is your command list\n";

	cout<<"\n";
	cout<<"login"<<"\n"
		<<"logout"<<"\n"
		<<"showAllBooks"<<"\n"
		<<"searchBook(for seeing details of a particular book {author,name,to whom book is issued.....})"<<"\n"
		<<"checkBookAvailability"<<"\n"
		<<"getIssuedBooks"<<"\n"
		<<"getYourInfo"<<"\n"
		<<"issueBook"<<"\n"
		<<"returnBook"<<"\n"
		<<"help"<<"\n";
	cout<<"------------------------------\n";
}
