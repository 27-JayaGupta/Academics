#pragma once
#include <bits/stdc++.h>
#include <fstream>
#include <string>
#include <iostream>
using namespace std;

class User{
	public:
		string id;
		string auth_token;

	public:
		void login_User(string role);
		void logout_User();
		void checkBookAvailability();
		void getIssuedBooks();
		void getCompleteUserInfo();
};