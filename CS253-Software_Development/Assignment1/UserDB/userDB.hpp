#pragma once
#include<bits/stdc++.h>
#include<iostream>
#include<string>
using namespace std;

class userDatabase{
	typedef struct User_in_DB{
		string id;
		string name;
		string password;
		string role;
		int number_books;
		int dues;
	}User_in_DB;

	public:
		vector<User_in_DB> userList;

	public:
		void addUser(string token);
		void showAllUsers(string token);
		void updateUser(string token);
		void deleteUser(string token);
		void searchUser(string token);
};