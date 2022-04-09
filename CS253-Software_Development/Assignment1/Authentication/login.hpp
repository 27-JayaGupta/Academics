#pragma once
#include <bits/stdc++.h>
#include <iostream>
#include <fstream>
#include <string>
using namespace std;

string token;
//users login and logout functionality are present in USER Class.
string generateToken(size_t length){

	auto randchar = []() -> char
    {
        const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
        const size_t max_index = (sizeof(charset) - 1);
        return charset[ rand() % max_index ];
    };
   	string str(length,0);
    generate_n( str.begin(), length, randchar );
    return str;
}

void login_Librarian(){
	
	if(!(token.empty())){
		cout<<"One user already logged in ... Kindly logout to login as another user\n";
		return;
	}

	string username,password;
	cout<<"------------------------------\n";
	cout<<"Enter username:";
	cin>>username;
	cout<<"Enter password:";
	cin>>password;

	if(username == "lib1" && password == "123"){
		cout<<"------------------------------\n";
		cout<<"Login Successful\n";
		token = generateToken(32);
		return;
	}	
	cout<<"------------------------------\n";
	cout<<"Login Failed..Enter correct credentials\n";
	return;
}

void logout_Librarian(){
	token.clear();
	cout<<"------------------------------\n";
	cout<<"Logged out Successfully\n";
	return;
}