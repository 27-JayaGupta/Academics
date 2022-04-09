#pragma once
#include <bits/stdc++.h>
#include <iostream>
#include <string>
using namespace std;

class Librarian{

	public:

		Librarian(){
			cout<<"-------------------\n";
			cout<<"Enter help for information on more commands\n";
		}

		void help(){
			cout<<"----------------------------\n";
			cout<<"Here is your command list\n";

			cout<<"\n";
			cout<<"login"<<"\n"
				<<"logout"<<"\n"
				<<"showAllBooks"<<"\n"
				<<"addBook"<<"\n"
				<<"searchBook(for seeing details of a particular book {author,name,to whom book is issued.....})"<<"\n"
				<<"updateBook"<<"\n"
				<<"deleteBook"<<"\n"
				<<"showAllUsers"<<"\n"
				<<"addUser"<<"\n"
				<<"searchUser(for seeing details of user{name,id,book issued,dues......})"<<"\n"
				<<"updateUser"<<"\n"
				<<"deleteUser"<<"\n"
				<<"clearFineAmount"<<"\n"
				<<"help"<<"\n";
			cout<<"------------------------------\n";
		}

		void clearFineAmount(){
			if(token.empty()){
		        cout<<"------------------------------\n";
		        cout<<"User not logged in .... Kindly login to access the functionaliy\n";
		        return ;
		    }

			string id;
			cout<<"Enter id of user you want to clear fine:";
			cin>>id;

			ifstream fin1("UserDB/userEntries/"+string(id)+".csv");

			if(fin1.fail()){
				cout<<"User does not exist\n";
				return;
			}

			vector<string> entry;
			string a;
			while (!fin1.eof()) {
				fin1>>a;
				entry.push_back(a);
				if (fin1.eof())
		    	break;
			}

			fin1.close();
			remove(("UserDB/userEntries/"+string(id)+".csv").c_str());


			fstream fout;
			fout.open("UserDB/userEntries/"+string(id)+".csv",ios::app);

			int amount;
			cout<<"Enter how much amount to clear(integer):";
			cin>>amount;

			for(int i=0;i<entry.size()-1;i++){
				if(i==5) fout<<to_string(stoi(entry[5]) - amount)<<"\n";
				else fout<<entry[i]<<"\n";
			}

		    fout.close();

		    cout<<"Fine cleared\n";			
		}

};