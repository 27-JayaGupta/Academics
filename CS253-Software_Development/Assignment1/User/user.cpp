#pragma once
#include "user.hpp"
#define BASE_PATH_BOOK "Books_Management/"
#define BASE_PATH_USER "UserDB/userEntries/"

string generateTokenUser(size_t length){

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

void User::login_User(string role){

	if(!(auth_token.empty())){
		cout<<"One user already logged in ... Kindly logout to login as another user\n";
		return;
	}

	string password,password_org;

	cout<<"Enter user id(only alphanumeric characters and underscore allowed):";
	cin>>id;
	cout<<"Enter Password(only alphanumeric characters and underscore allowed):";
	cin>>password;

	ifstream fin("UserDB/userEntries/"+string(id)+string(".csv"));

	if(fin.fail()){
		cout<<"User does not exist...\n";
		return ;
	}

    string var;
    int index = 0;
    while(!fin.eof()){
		fin>>var;
		if((index) ==2) {
			password_org = var;
		}

		if(index == 3){
			if(var != role){
				cout<<"Invalid Login...Enter correct credentials\n";
				return;
			}
		}

		index++ ;
    	if(fin.eof()) break;
    }

    fin.close();

    if(password != password_org){
    	cout<<"Invalid credentials!\n";
    	return;
    }

    cout<<"------------------------------\n";
	cout<<"Login Successful\n";
	auth_token = generateTokenUser(32);
	return;
}

void User::logout_User(){
	auth_token.clear();
	cout<<"------------------------------\n";
	cout<<"Logged out Successfully\n";
	return;
}

void User::checkBookAvailability(){

	if(auth_token.empty()){
        cout<<"------------------------------\n";
        cout<<"User not logged in .... Kindly login to access the functionaliy\n";
        return ;
    }

	ifstream fin("Books_Management/books.csv");

    if(fin.fail()){
        cout<<"\nError opening File\n";
        return;
    }

	string isbn,isbn1;
	string line, word;
	vector<string> row;

	cout << "Enter the ISBN number "
	     << "of the book you want to check Availability:";
	cin >> isbn;

	int index = 0;
		int flag = 0;	
	while (!fin.eof()) {

		row.clear();
  
       	fin>>line;
        stringstream s(line);
  
        while (s.good()) {
	        string substr;
	        getline(s, substr, ',');
	        row.push_back(substr);
		}
  
        isbn1 = row[0];
        int row_size = row.size();

    	if ((isbn == isbn1) && !flag) {
            flag = 1;
            cout<<"Record Found\n";
            cout<<"Availability Status: ";
            if(row[4]=="no") cout<<"YES\n";
            else cout<<"NO\n";
            
            break;
    	}

    	if (fin.eof())
        	break;
	}

    if (flag == 0)
        cout << "Record not found\n";

    // Close the pointers
    fin.close();
}

void User::getIssuedBooks(){

	if(auth_token.empty()){
        cout<<"------------------------------\n";
        cout<<"User not logged in .... Kindly login to access the functionaliy\n";
        return ;
    }

	ifstream fin("UserDB/userEntries/"+string(id)+".csv");
	if(fin.fail()){
		cout<<"Error opening file......Either the user does not exist or there is some issue\n";
		return;
	}

    string var;
    int index = 0;
    cout<<"Id of Books Issued:\n";
    while(!fin.eof()){
    	fin>>var;
    	if(fin.eof()) break;
    	if(index++ < 6) continue;
    	cout<<var<<"\n";
    }

    if(index == 6){
    	cout<<"No books Issued\n";
    	cout<<"\n";
    }

  	fin.close();

}		

void User::getCompleteUserInfo(){
	if(auth_token.empty()){
        cout<<"------------------------------\n";
        cout<<"User not logged in .... Kindly login to access the functionaliy\n";
        return ;
     }
    
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

    	cout<<var<<"\n";
    }

    cout<<"\n";
  	
  	fin.close();
}