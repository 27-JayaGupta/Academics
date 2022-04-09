#pragma once
#include "userDB.hpp"
#include<fstream>

using namespace std;

void userDatabase::addUser(string token){

	if(token.empty()){
        cout<<"------------------------------\n";
        cout<<"User not logged in .... Kindly login to access the functionaliy\n";
        return ;
    	}

	User_in_DB temp;
	cout<<"Enter id number(or roll_no):";
	cin >> temp.id;
	cout<<"Enter User Name (no space in between):";
	cin >>temp.name;
	cout << "Add User Password(default) (no space in between):";
	cin >> temp.password;
	cout << "Add user Role (no space in between):";
	cin >> temp.role;
	temp.number_books= 0;
	temp.dues = 0;

	ifstream fin("UserDB/userEntries/"+string(temp.id)+".csv");
	if(!fin.fail()){
		cout<<"User already exists..\n";
		return;
	}

	fstream fout;
	fstream fout1;

    // opens an existing csv file or creates a new file.
    fout.open("UserDB/userEntries/"+string(temp.id)+".csv", ios::app);
    fout1.open("UserDB/userEntries/"+string("user_id.csv"),ios::app);

    // Insert the data to file
    fout <<temp.id << "\n"
         << temp.name<< "\n"
         << temp.password << "\n"
         << temp.role << "\n"
         << temp.number_books << "\n"
      	 << temp.dues
         << "\n";

    fout1 << temp.id<<"\n";
    fout1.close();
    fout.close();

    cout<<"User added successfully\n";
}

void userDatabase::showAllUsers(string token){

	if(token.empty()){
        cout<<"------------------------------\n";
        cout<<"User not logged in .... Kindly login to access the functionaliy\n";
        return ;
    	}

	fstream fin("UserDB/userEntries/user_id.csv");
	if(fin.fail()){
		cout<<"Error in openig file\n";
		return;
	}

	string id;
	int index ;
	while(!fin.eof()){
		
		fin>>id;
		if (fin.eof())
        	break;
		
		fstream fin1;
		fin1.open("UserDB/userEntries/"+string(id)+".csv",ios::in);

		vector<string> row;
	    	string temp;
	    	cout<<"-----------------USER-------------------\n";
	    	index = 0;
	    	while (fin1 >> temp) {
        		row.clear();
        		if((index) == 0) {
        			cout<<"ID: ";
        			index ++ ;
        		}
		    	else if((index) == 1) {
		    		cout<<"Name: ";
		    		index ++ ;
		    	}
		    	else if((index) == 2) {
		    		cout<<"Password: ";
		    		index ++ ;
		    	}
		    	else if((index) == 3) {
		    		cout<<"Role: ";
		    		index ++ ;
		    	}
		    	else if((index) == 4) {
		    		cout<<"Number of Books Issued: ";
		    		index ++ ;
		    	}
		    	else if((index) == 5 ) {
		    		cout<<"Fine: ";
		    		index ++ ;
		    	}
		    	else if((index) == 6) {
		    		cout<<"Books ID Issued\n:";
		    		index ++ ;
		    	}
		    	
  
	        // used for breaking words
	        stringstream s(temp);

 		   cout<<temp<<"\n";
    		}

    	cout<<"\n";
	}

	fin.close();
}

void userDatabase::updateUser(string token){

	if(token.empty()){
        cout<<"------------------------------\n";
        cout<<"User not logged in .... Kindly login to access the functionaliy\n";
        return ;
    	}

	string id;
	cout<<"Enter id of user to be updated(only alphanumeric char and underscore):";
	cin>>id;

	ifstream fin("UserDB/userEntries/"+string(id)+".csv");
	fstream fout;

	if(fin.fail()){
		cout<<"Error opening file......Either the user does not exist or there is some issue\n";
		return;
	}
	
	fout.open("UserDB/userEntries/"+string(id)+"new.csv", ios::app);
    vector<string> var;
    string password;
    while(!fin.eof()){
    	string s;
    	fin>>s;
    	var.push_back(s);
    }
    User_in_DB temp;
    temp.id = id;
    cout<<"Enter name (if to be updated)(no space in between) :";
    	cin >>temp.name;
    	cout << "Add role(if to be updated)(no space in between) :";
    	cin >> temp.role;
	
  	
  	fout<<temp.id<<"\n"
  		<<temp.name<<"\n"
  		<<var[2]<<"\n"
  		<<temp.role<<"\n";

  	for(int i=4;i<var.size()-1;i++){
  		fout<<var[i]<<"\n";
  	}
  	
  	fin.close();
  	fout.close();

    // removing the existing file
    remove(("UserDB/userEntries/"+string(id)+string(".csv")).c_str());
  
    // renaming the updated file with the existing file name
    rename(("UserDB/userEntries/"+string(id)+string("new.csv")).c_str(), ("UserDB/userEntries/"+string(id)+string(".csv")).c_str());
    cout<<"User updated successfully\n";
    cout<< "Updated Entry : ";
    cout <<"ID: "<<id << "\n"
         << "Name: "<<temp.name<< "\n"
         << "Role:"<<temp.role << "\n";
}

void userDatabase::deleteUser(string token){

	if(token.empty()){
        cout<<"------------------------------\n";
        cout<<"User not logged in .... Kindly login to access the functionaliy\n";
        return ;
     }

	string id;
	cout<<"Enter id of user to be deleted(only alphanumeric char and underscore):";
	cin>>id;

	ifstream fin("UserDB/userEntries/"+string(id)+".csv");
	if(fin.fail()){
		cout<<"Error opening file......Either the user does not exist or there is some issue\n";
		return;
	}

	remove(("UserDB/userEntries/"+string(id)+string(".csv")).c_str());
	fin.close();

	fstream fin1,fout;
	fin1.open("UserDB/userEntries/user_id.csv",ios::in);
	fout.open("UserDB/userEntries/"+string("user_id_new.csv"), ios::app);
    	string var;
    	while(!fin1.eof()){
    		fin1>>var;
    		if(var == id) continue;
    		fout<<var<<"\n";
    	}
  	
  	fin1.close();
  	fout.close();

    // removing the existing file
    remove("UserDB/userEntries/user_id.csv");
  
    // renaming the updated file with the existing file name
    rename("UserDB/userEntries/user_id_new.csv", "UserDB/userEntries/user_id.csv");

    cout<<"User deleted successfully\n";
}

void userDatabase::searchUser(string token){

	if(token.empty()){
        cout<<"------------------------------\n";
        cout<<"User not logged in .... Kindly login to access the functionaliy\n";
        return ;
     }

	string id;
	cout<<"Enter id of user you want to search(only alphanumeric char and underscore):";
	cin>>id;

	ifstream fin("UserDB/userEntries/"+string(id)+".csv");
	if(fin.fail()){
		cout<<"Error opening file......Either the user does not exist or there is some issue\n";
		return;
	}

    string var;
    int index = 0;
    while(!fin.eof()){
    	fin>>var;
    	if(fin.eof()) break;
    	if(index == 0)cout<<"ID:"<<var<<"\n";
    	if(index == 1) cout<<"Name:" <<var<<"\n";
    	if(index ==3) cout<<"Role:" <<var<<"\n";
    	if(index ==4) cout<<"Number of books issued: " <<var<<" \n";
    	if(index ==5) cout <<"Fine Amount: "<<var<<"\n";
    	if(index == 6) {
    		cout<<"Books ID Issued:\n";
    		cout<<var<<" ";
    	}
    	if(index > 6) cout<<var<<" ";
    	index ++;
    }

    cout<<"\n";
  	
  	fin.close();
}
