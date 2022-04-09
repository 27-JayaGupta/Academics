# Sudoku pair: solving and generation
## Question:
* This assignment has two questions to be solved. You are expected to submit code for both Q.1 & Q.2. 
You should also submit a brief write-up on your approach to solving these questions.

* In this question, you have to write a k-sudoku puzzle pair solver and generator by encoding the problem to propositional logic and solving it via a SAT solver (https://pysathq.github.io/). 

### Question 1 

Given a sudoku puzzle pair S1, S2 (both of dimension k) as input, your job is to write a program to fill the empty cells of both sudokus such that it satisfies the following constraints
* Individual sudoku properties should hold.
* For each empty cell S1[i, j] ≠ S2[i, j], where i is row and j is column.

#### Input: 
Parameter k, single CSV file containing two sudokus. The first k * k rows are for the first sudoku and the rest are for the second sudoku. Each row has k * k cells. Each cell contains a number from 1 to k * k. Cell with 0 specifies an empty cell.
Output: If the sudoku puzzle pair doesn't have any solution, you should return None otherwise return the filled sudoku pair.

### Question 2
In the second part, you have to write a k-sudoku puzzle pair generator. The puzzle pair must be maximal (have the largest number of holes possible) and must have a unique solution. 
* Input: Parameter k
* Output: CSV file containing two sudokus in the format mentioned in Q1.

## Deliverables
The source code of your implementation. 
A brief report (less than 5-pages) describing your implementation, assumptions, and limitations.
A set of test cases (at least 5) with the expected output. (tests folder).
The quality of all the above would affect your marks. The quality of all the above would affect your marks.

## Submission Format
Your submission MUST be in the following format:

The submission should be a zip file.
The zip file should be named as assignment_"number"_"Roll-of-student1"_"Roll-of-student2".
Zip the content of the source as is and submit. 
Please note that your submission will NOT be graded if you do not follow the format. Furthermore, we will use the Readme file provided by you to build and run your code. Therefore, please make sure that the Readme is clear. We cannot grade your submission if we cannot run it on our system.
