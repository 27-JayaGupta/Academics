#pragma once
#include "book.hpp"
#include<fstream>
#include <ctime>

using namespace std;

void Book::issueBook(string role,string id,string auth_token){

	if(auth_token.empty()){
        cout<<"------------------------------\n";
        cout<<"User not logged in .... Kindly login to access the functionaliy\n";
        return ;
    }

	string isbn,isbn1;
	cout<<"Enter book ISBN number you want to issue (only alphanumeric characters and underscore):";
	cin>>isbn;

	fstream fin,fout;

	ifstream fin1("UserDB/userEntries/"+string(id)+".csv");
	if(fin1.fail()){
		cout<<"Error opening file\n";
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

	if((stoi(entry[4]) == 5) && (role == "student")){
		cout<<"You have already issued 5 books....You cant issue more books\n";
		return;
	}
 	fin1.close();

	fin.open("Books_Management/books.csv", ios::in);
	fout.open("Books_Management/booksnew.csv",ios::app);
	string line, word;
	vector<string> row;

	int flag = 0;
	int alreadyIssued = 0;
	long m,d,y;	
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
            if(row[4]=="yes"){
            	cout<<"-------------------------------\n";
            	cout<<"Book already Issued\n";
            	alreadyIssued = 1;
            	break;
            }
            
            for(int i=0;i<row.size();i++){
            	if(i<=3) {
            		fout<< row[i]<< ",";
            		continue;
            	}

            	if(i==4){
            		fout << "yes,";
            		continue;
            	}
	
        		if(i==5){
        			time_t t = time(0);   // get time now
					tm* now = std::localtime(&t);
					y = now->tm_year + 1900;
					m = now->tm_mon + 1 ;
					d = now->tm_mday;
					fout << to_string(now->tm_year + 1900) << '-' 
							         << to_string(now->tm_mon + 1) << '-'
							         <<  to_string(now->tm_mday)
							         << ",";
        		}
        		else if(i==6){
        			long num_day;
        			m = (m + 9) % 12;
					y = y - m/10;
					num_day = 365*y + y/4 - y/100 + y/400 + (m*306 + 5)/10 + ( d - 1 );

					if(role=="student") num_day += 30;
					else if(role=="professor") num_day +=60;
        			
        			y = (10000*num_day + 14780)/3652425 ;
					long long ddd = num_day - (365*y + y/4 - y/100 + y/400) ;
					if (ddd < 0) {
						y = y - 1 ;
						ddd = num_day - (365*y + y/4 - y/100 + y/400) ;
					}
					long long mi = (100*ddd + 52)/3060 ;
					m = (mi + 2)%12 + 1 ;
					y = y + (mi + 2)/12;
					d = ddd - (mi*306 + 5)/10 + 1;

					fout<<to_string(y)<<'-'<<to_string(m)<<'-'<<to_string(d)<<',';
        		}
        		else if(i==7){
        			fout<<id<<"\n";
        		}
            	
            }
    	}
    	else{
    		if (!fin.eof()) {
                for (int i = 0; i < row_size - 1; i++) {
                    fout << row[i] << ",";
                }
                fout << row[row_size - 1] << "\n";
            }
    	}

    	if (fin.eof())
        	break;
	}

    if (flag == 0){
    	cout<<"---------------------------------------------\n";
        cout << "Record not found\n";
    }

    // Close the pointers
    fin.close();
    fout.close();
    	
    if(flag && (!alreadyIssued)){
    	ifstream fin1("UserDB/userEntries/"+string(id)+".csv");
		if(fin1.fail()){
			cout<<"Error opening file\n";
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
    	//cout<<"here\n";
    	fout.open("UserDB/userEntries/"+string(id)+".csv",ios::app);
    	for(int i=0;i<entry.size()-1;i++){
    		if(i==4) {
    			fout<< to_string(stoi(entry[i])+1)<<"\n";
    			continue;
    		}
    		fout<<entry[i]<<"\n";
    	}
    	fout<<isbn<<"\n";
    	fout.close();
    	cout<<"Book Issued successfully\n";
    }

    if(alreadyIssued){
    	remove("Books_Management/booksnew.csv");
    }
    else{

	    // removing the existing file
	    remove("Books_Management/books.csv");
	  
	    // renaming the new file with the existing file name
	    rename("Books_Management/booksnew.csv", "Books_Management/books.csv");
	}
}

void Book::returnBook(string role,string id,string auth_token){

	if(auth_token.empty()){
        cout<<"------------------------------\n";
        cout<<"User not logged in .... Kindly login to access the functionaliy\n";
        return ;
    }

    int fine = 0;

	string isbn,isbn1;
	cout<<"Enter book ISBN number you want to return(only alphanumeric characters and underscore) :";
	cin>>isbn;

	fstream fin,fout;
	fin.open("Books_Management/books.csv", ios::in);
	fout.open("Books_Management/booksnew.csv",ios::app);
	string line, word;
	vector<string> row;

	int flag = 0;
	int check = 0;
	long m,d,y;	
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
            if(row[4]=="no"){
            	cout<<"Book not issued \n";
            	check = 1;
            	break;
        	}

            string id1 =  row[7];

            if(id!=id1){
            	cout<<"Book not issued on your name\n";
            	check = 1;
            	break;
            }
            stringstream X(row[5]);
            string date,month,year;
            getline(X,year,'-');
        	getline(X,month,'-');
        	getline(X,date,'-');

        	long y,d,m;
        	d = stol(date);
        	m = stol(month);
        	y = stol(year);
        	long num_day;
			m = (m + 9) % 12;
			y = y - m/10;
			num_day = 365*y + y/4 - y/100 + y/400 + (m*306 + 5)/10 + ( d - 1 );
        	//cout<<year<<" "<<month<<" "<<date<<"\n";

        	time_t ttime = time(0);
			tm *current_time = localtime(&ttime);
			current_time->tm_hour = 0;
			current_time->tm_min = 0;
			current_time->tm_sec = 0;

			long num_day2;
			long dd,mm,yy;
			dd=(long)(current_time->tm_mday);
			mm=(long)(current_time->tm_mon) + 1;
			yy=(long)(current_time->tm_year)+1900;
			//cout<<dd<<" "<<mm<<" "<<yy<<"\n";
			mm = (mm + 9) % 12;
			yy = yy - mm/10;
			num_day2 = 365*yy + yy/4 - yy/100 + yy/400 + (mm*306 + 5)/10 + ( dd - 1 );

			int days = num_day2 - num_day;
			//cout<<days<<"\n";
            if(role == "student"){
				if(days >= 30)
					fine += 2 * (days-30+1);
			}
			else if(role == "professor"){
				if(days >= 60)
					fine += 5 * (days-60+1);
			}
            
        
            for(int i=0;i<row.size();i++){
            	if(i<=3) {
            		fout<< row[i]<< ",";
            		continue;
            	}

            	if(i==4){
            		fout << "no,";
            		continue;
            	}

            	if(i==7){
            		fout<<"nil\n";
            		continue;
            	}

            	fout<<"nil,";
            }
    	}
    	else{
    		if (!fin.eof()) {
                for (int i = 0; i < row_size - 1; i++) {
                    fout << row[i] << ",";
                }
                fout << row[row_size - 1] << "\n";
            }
    	}

    	if (fin.eof())
        	break;
	}

    if (flag == 0){
    	cout<<"---------------------------------------------\n";
        cout << "Record not found\n";
    }

    // Close the pointers
    fin.close();
    fout.close();

    if(!check && flag){
    	ifstream fin1("UserDB/userEntries/"+string(id)+".csv");
		if(fin1.fail()){
			cout<<"Error opening file\n";
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
    	//cout<<"here\n";
    	fout.open("UserDB/userEntries/"+string(id)+".csv",ios::app);
    	for(int i=0;i<entry.size()-1;i++){
    		if(i==4) {
    			fout<< to_string(stoi(entry[i])-1)<<"\n";
    			continue;
    		}
    		if( i==5){
    			fout<<to_string(fine)<<"\n";
    			continue;
    		}
    		if(i>5 && isbn==entry[i]) continue;
    		fout<<entry[i]<<"\n";
    	}
    	fout.close();

    	cout<<"Book Returned successfully\n";
    }
    	
    if(!check){
	    // removing the existing file
	    remove("Books_Management/books.csv");
	  
	    // renaming the new file with the existing file name
	    rename("Books_Management/booksnew.csv", "Books_Management/books.csv");
    }
    else{
    	remove("Books_Management/booksnew.csv");
    }
}