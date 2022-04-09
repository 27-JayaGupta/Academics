#pragma once
#include"booksDB.hpp"
#include<fstream>
using namespace std;

void booksDatabase::showAllBooks(string token){

    if(token.empty()){
        cout<<"------------------------------\n";
        cout<<"User not logged in .... Kindly login to access the functionaliy\n";
        return ;
    }

    ifstream fin("Books_Management/books.csv");
  
    // Open an existing file
    if(fin.fail()){
        cout<<"------------------------------\n";
        cout<<"Error opening file.....\n";
        return;
    }
  
    // Read the Data from the file
    // as String Vector
    vector<string> row;
    string line, word, temp;
  	allBooks.clear();
    while (fin >> temp) {
        row.clear();
  
        // used for breaking words
        stringstream s(temp);

        while (s.good()) {
	        string substr;
	        getline(s, substr, ',');
	        row.push_back(substr);
	    }

  		Book temp;
	    temp.ISBN = row[0];
	    temp.Title = row[1];
        temp.Author = row[2];
		temp.Publication = row[3];
		temp.isPublished = row[4];
		temp.PublishDate = row[5];
		temp.DueDate = row[6];
		temp.IssuedTo = row[7];

		allBooks.push_back(temp);
    }
    fin.close();

	//cout<<"ISBN    Title      Author     Publication              isPublished    PublishDate     DueDate      IssuedTo       \n";
	cout<<"------------------------------\n";
    cout<<"Books Details \n \n";
    for(int i=0;i<allBooks.size();i++){
        cout<<"-----------------Book--------------------\n";
		cout<<"ISBN:"<<allBooks[i].ISBN<<"\n"
			<<"Title:"<<allBooks[i].Title<<"\n"
			<<"Author:"<<allBooks[i].Author<<"\n"
			<<"Publication:"<<allBooks[i].Publication<<"\n"
			<<"Issued Status(yes or no):"<<allBooks[i].isPublished<<"\n"
			<<"Publish Date:"<<allBooks[i].PublishDate<<"\n"
			<<"DueDate:"<<allBooks[i].DueDate<<"\n"
			<<"Issued To:"<<allBooks[i].IssuedTo<<"\n";
	}
    return;
}

void booksDatabase::addBook(string token){

    if(token.empty()){
        cout<<"------------------------------\n";
        cout<<"User not logged in .... Kindly login to access the functionaliy\n";
        return ;
    }
    cout<<"------------------------------\n";
	Book temp;
	cout<<"Enter ISBN number(only alphanumeric characters and underscore):";
	cin >> temp.ISBN;
	cout<<"Enter Book Title (no space in between):";
	cin >>temp.Title;
	cout << "Add Book Author (no space in between):";
	cin >> temp.Author;
	cout << "Add Book Publication (no space in between):";
	cin >> temp.Publication;
    cout<<"------------------------------\n";
	temp.isPublished = "no";
	temp.PublishDate = "nil";
	temp.DueDate = "nil";
	temp.IssuedTo = "nil";

	fstream fout;
    fout.open("Books_Management/books.csv",ios::app);

    // Insert the data to file
    fout << temp.ISBN << ","
         << temp.Title<< ","
         << temp.Author << ","
         << temp.Publication << ","
         << temp.isPublished << ","
      	 << temp.PublishDate << ","
         << temp.DueDate << ","
         <<temp.IssuedTo
         << "\n";

    fout.close();
    cout<<"\n";
    cout<<"Book Added Successfully\n";
}

void booksDatabase::updateBook(string token){

    if(token.empty()){
        cout<<"------------------------------\n";
        cout<<"User not logged in .... Kindly login to access the functionaliy\n";
        return ;
    }

    ifstream fin("Books_Management/books.csv");
    if(fin.fail()){
        cout<<"\n Error opening File\n";
        return;
    }
	fstream fout;
    fout.open("Books_Management/booksnew.csv" ,ios::app);

    string isbn,isbn1;
    string line, word;
    vector<string> row;
  
   
    cout << "Enter the ISBN number "
         << "of the record to be updated:";
    cin >> isbn;
    
  	int index = 0;
  	int flag = 0;
    Book temp;
    // Traverse the file
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

            
            temp.ISBN = isbn;
            cout<<"Enter updated Book Title (no space in between):";
            cin >>temp.Title;
            cout << "Add updated Book Author (no space in between):";
            cin >> temp.Author;
            cout << "Add updated Book Publication (no space in between):";
            cin >> temp.Publication;

            stringstream convert;
  
            // sending a number as a stream into output string
            convert << isbn;
            row[index++] = convert.str();

            convert.str("");
            convert << temp.Title;
            row[index++] = convert.str();

            convert.str("");
            convert << temp.Author;
            row[index++] = convert.str();

            convert.str("");
            convert << temp.Publication;
            row[index++] = convert.str();

            convert.str("");
            convert << row[index];
            temp.isPublished = row[index];
            row[index++] = convert.str();

            convert.str("");
            convert << row[index];
            temp.PublishDate = row[index];
            row[index++] = convert.str();

            convert.str("");
            convert << row[index];
            temp.DueDate = row[index];
            row[index++] = convert.str();

            convert.str("");
            convert << row[index];
            temp.IssuedTo = row[index];
            row[index++] = convert.str();
  
            if (!fin.eof()) {
                for (int i = 0; i < row_size - 1; i++) {
  
                    // write the updated data
                    // into a new file 'reportcardnew.csv'
                    // using fout
                    fout << row[i] << ",";
            }
  
                fout << row[row_size - 1] << "\n";
            }
        }
        else {
            if (!fin.eof()) {
                for (int i = 0; i < row_size - 1; i++) {
  
                    // writing other existing records
                    // into the new file using fout.
                    fout << row[i] << ",";
                }
  
                // the last column data ends with a '\n'
                fout << row[row_size - 1] << "\n";
            }
        }

        if (fin.eof())
            break;
    }

    if (flag == 0){
        cout << "Record not found\n";
    }
  
    // removing the existing file
    remove(("Books_Management/books.csv"));
  
    // renaming the updated file with the existing file name
    rename(("Books_Management/booksnew.csv"), ("Books_Management/books.csv"));

    if(flag){
        cout<< "Updated Entry : ";
        cout<<"ISBN:"<<temp.ISBN<<"\n"
            <<"Title:"<<temp.Title<<"\n"
            <<"Author:"<<temp.Author<<"\n"
            <<"Publication:"<<temp.Publication<<"\n"
            <<"Issued Status(yes or no):"<<temp.isPublished<<"\n"
            <<"Publish Date:"<<temp.PublishDate<<"\n"
            <<"DueDate:"<<temp.DueDate<<"\n"
            <<"Issued To:"<<temp.IssuedTo<<"\n";
     }

    fout.close();

}

void booksDatabase::deleteBook(string token){

    if(token.empty()){
        cout<<"------------------------------\n";
        cout<<"User not logged in .... Kindly login to access the functionaliy\n";
        cout<<"------------------------------\n";
        return ;
    }
	
    ifstream fin("Books_Management/books.csv");
    if(fin.fail()){
        cout<<"\n Error opening File\n";
        return;
    }
	fstream  fout;
	fout.open("Books_Management/booksnew.csv", ios::out);

	string isbn,isbn1;
	string line, word;
	vector<string> row;

	cout << "Enter the ISBN number "
	     << "of the record to be deleted(only alphanumeric characters and underscore):";
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

    	if (isbn != isbn1) {
            if (!fin.eof()) {
                for (int i = 0; i < row_size - 1; i++) {
                    fout << row[i] << ",";
                }
                fout << row[row_size - 1] << "\n";
            }
    	}
    	else {
        	flag = 1;
    	}

    	if (fin.eof())
        	break;
	}

    if (flag == 1)
        cout << "Record deleted\n";
    else
        cout << "Record not found\n";

    // Close the pointers
    fin.close();
    fout.close();

    // removing the existing file
    remove(("Books_Management/books.csv"));
  
    // renaming the new file with the existing file name
    rename(("Books_Management/booksnew.csv"), ("Books_Management/books.csv"));
}

void booksDatabase::searchBook(string token){

    if(token.empty()){
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
	     << "of the record to be searched:";
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
            cout<<"Details: \n";
            cout<<"ISBN:"<<row[0]<<"\n"
            <<"Title:"<<row[1]<<"\n"
            <<"Author:"<<row[2]<<"\n"
            <<"Publication:"<<row[3]<<"\n"
            <<"Issued Status(yes or no):"<<row[4]<<"\n"
            <<"Publish Date:"<<row[5]<<"\n"
            <<"DueDate:"<<row[6]<<"\n"
            <<"Issued To:"<<row[7]<<"\n";
            cout<<"\n";
    	}

    	if (fin.eof())
        	break;
	}

    if (flag == 0)
        cout << "Record not found\n";

    // Close the pointers
    fin.close();
}
