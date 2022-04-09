#pragma once
#include<bits/stdc++.h>
#include<iostream>
#include<string>
#include <sstream>
#include <ctime>
using namespace std;

class Book{

	public:
		string ISBN;
		string Title;
		string Author;
		string Publication;
		string isPublished;
		string PublishDate;
		string DueDate;
		string IssuedTo;

	public:
		void issueBook(string role,string id,string auth_token);
		void returnBook(string role,string id,string auth_token);     // update fine on user 
};
