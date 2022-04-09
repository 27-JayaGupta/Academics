### Instruction to run the program

The source file for execution of the code is main.cpp .Compile main.cpp and execute it to run the program .

```
g++ main.cpp
./a.out
```

### Some data required to login and access the functionality

* There are three users(professor,student,librarian). You need to select a valid role to begin.
* ```help``` command is available for all the three users which lists down all the functionality avaialable to the users.
* In order to access any other functionality , login is required first.
* Login Credentials:
	* Librarian : ```Username : lib1 , Password: 123```
* Librarian has the authority to create new users. So the id and password of the new user is set by the librarian which are used by the respective users to further login.
* The class structure is similar to the diagram provided in the problem statement with few modifications.
	* Professor and student class are not given clear_fine_amount functionality. The function for the same is made in librarian class.
	* Librarian class is not inherited from User Class as it has no property common with the User Class.
	* ```calculate_penalty``` functionality is added in returnBook function in Book Class.It takes the role of the user annd the return date and calculates fine accordingly.

* For clear understanding of the class structure, every cpp file has a corresponding hpp file which only contains the variable and method declarations.
* Common  functionalities for student and professor(like showBookAvailability,showYourPersonalInfo,..) are implemented in the ```User``` class.

* I have implemented a file system where there is no need for the librarian to input data on each run of program. The data is stored through multiple different sessions.The data is read and stored from the files.