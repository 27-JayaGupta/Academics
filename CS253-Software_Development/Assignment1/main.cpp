#include<bits/stdc++.h>
#include<iostream>
#include "UserDB/userDB.cpp"
#include "Books_Management/booksDB.cpp"
#include "Books_Management/book.cpp"
#include "Authentication/login.hpp"
#include "User/librarian.hpp"
#include "User/Professor.cpp"
#include "User/Student.cpp"
using namespace std;

int main(){

	string role;
	string command;
	cout<<"Enter you role(librarian,student,professor): ";
	cin>>role;

	if(role=="librarian"){
		Librarian lib;
		booksDatabase b;
		userDatabase u;
		//Librarian object here
		while(1){
			cout<<"------------------------------\n";
			cout<<"Enter command:";
			cin>>command;

			if(command == "help"){
				lib.help();
				continue;
			}
			else if(command == "login"){
				cout<<"Default username: lib1 \n";
				cout<<"Default Password : 123 \n"; 
				login_Librarian();
				continue;
			}
			else if(command == "showAllBooks"){
				b.showAllBooks(token);
				continue;
			}
			else if(command == "addBook"){
				b.addBook(token);
				continue;
			}
			else if(command == "updateBook"){
				b.updateBook(token);
				continue;
			}
			else if(command == "deleteBook"){
				b.deleteBook(token);
				continue;
			}
			else if(command == "searchBook"){
				b.searchBook(token);
				continue;
			}
			else if(command == "showAllUsers"){
				u.showAllUsers(token);
				continue;
			}
			else if(command == "addUser"){
				u.addUser(token);
				continue;
			}
			else if(command == "updateUser"){
				u.updateUser(token);
				continue;
			}
			else if(command == "searchUser"){
				u.searchUser(token);
				continue;
			}
			else if(command == "deleteUser"){
				u.deleteUser(token);
				continue;
			}
			else if(command == "clearFineAmount"){
				lib.clearFineAmount();
			}
			else if(command == "logout"){
				logout_Librarian();
				return 0;
			}
			else{
				cout<<"Invalid Command , enter commands from list (can see from help)\n";
				continue;
			}	
		}
	}
	else if(role=="student"){
		Student s;
		booksDatabase b;
		
		//Student object here
		while(1){
			cout<<"------------------------------\n";
			cout<<"Enter command:";
			cin>>command;

			if(command == "help"){
				s.help();
				continue;
			}
			else if(command == "login"){
				s.login_User("student");
				continue;
			}
			else if(command == "showAllBooks"){
				b.showAllBooks(s.auth_token);
				continue;
			}
			else if(command == "searchBook"){
				b.searchBook(s.auth_token);
				continue;
			}
			else if(command == "checkBookAvailability"){
				s.checkBookAvailability();
				continue;
			}
			else if(command == "getYourInfo"){
				s.getCompleteUserInfo();
				continue;
			}
			else if(command == "getIssuedBooks"){
				s.getIssuedBooks();
				continue;
			}
			else if(command == "issueBook"){
				b.issueBook("student",s.id,s.auth_token);
				continue;
			}
			else if(command == "returnBook"){
				b.returnBook("student",s.id,s.auth_token);
				continue;
			}
			else if(command == "logout"){
				s.logout_User();
				return 0;
			}
			else{
				cout<<"Invalid Command , enter commands from list (can see from help)\n";
				continue;
			}
		}
	}
	else if(role=="professor"){
		Professor p;
		booksDatabase b;
		userDatabase u;
		//Student object here
		while(1){
			cout<<"------------------------------\n";
			cout<<"Enter command:";
			cin>>command;

			if(command == "help"){
				p.help();
				continue;
			}
			else if(command == "login"){
				p.login_User("professor");
				continue;
			}
			else if(command == "showAllBooks"){
				b.showAllBooks(p.auth_token);
				continue;
			}
			else if(command == "searchBook"){
				b.searchBook(p.auth_token);
				continue;
			}
			else if(command == "checkBookAvailability"){
				p.checkBookAvailability();
				continue;
			}
			else if(command == "getYourInfo"){
				p.getCompleteUserInfo();
				continue;
			}
			else if(command == "getIssuedBooks"){
				p.getIssuedBooks();
				continue;
			}
			else if(command == "issueBook"){
				b.issueBook("professor",p.id,p.auth_token);
				continue;
			}
			else if(command == "returnBook"){
				b.returnBook("professor",p.id,p.auth_token);
				continue;
			}
			else if(command == "logout"){
				p.logout_User();
				return 0;
			}
			else{
				cout<<"Invalid Command , enter commands from list (can see from help)\n";
				continue;
			}
		}
	}
	
	cout<<"Enter a valid role to run the program\n";
	return 0;

}