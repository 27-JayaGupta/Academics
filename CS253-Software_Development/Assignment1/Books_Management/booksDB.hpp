#pragma once
#include<bits/stdc++.h>
#include<iostream>
#include<string>
#include <sstream>
#include "book.hpp"
using namespace std;

class booksDatabase:public Book{

	public:
		vector<Book> allBooks;

	public:
		void addBook(string token);
		void showAllBooks(string token);
		void updateBook(string token);
		void deleteBook(string token);
		void searchBook(string token);
};
