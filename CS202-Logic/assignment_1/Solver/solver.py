from pysat.formula import CNF
from pysat.solvers import Solver
from pprint import pprint
import csv
from time import time
class Sudoku_Solver:
    def __init__(self, k):
        self.k = k
        self.ksq = k**2
    def v(self, i, j, d):
        return (self.ksq**2)*(i-1) + self.ksq*(j-1) + d
    def v_pair(self, i, j, d, m): #m = 1 or 2
        return 2*((self.ksq**2)*(i-1) + self.ksq*(j-1) + d) + m
    def read_sudoku_pair(self):
        puzzle1 = []
        puzzle2 = []
        filename = "./tests/"+str(self.k) + "_empty.csv"
        # filename = "tests/3_nosolution.csv"
        # filename = "tests/4_nosolution.csv"
        with open(filename, mode='r') as f:
            csvreader = csv.reader(f)
            for line in csvreader:
                if len(puzzle1) < self.ksq:
                    puzzle1.append(line)
                else:
                    puzzle2.append(line)
        for i in range(0, self.ksq):
            for j in range(0, self.ksq):
                tmp = int(puzzle1[i][j])
                puzzle1[i][j] = tmp
                tmp = int(puzzle2[i][j])
                puzzle2[i][j] = tmp 
        return (puzzle1, puzzle2)
    def make_clause_pair(self):
        claus=[]
        for i in range(1, self.ksq+1):
            for j in range(1, self.ksq+1):
                claus.append([self.v_pair(i, j ,d, 1) for d in range(1, self.ksq+1)]) #each box should have atleast one number 
                claus.append([self.v_pair(i, j ,d, 2) for d in range(1, self.ksq+1)])
                for d in range(1, self.ksq):
                    for dp in range(d+1, self.ksq+1):
                        claus.append([-self.v_pair(i, j, d, 1), -self.v_pair(i, j, dp, 1)]) # each box should have only one number assigned
                        claus.append([-self.v_pair(i, j, d, 2), -self.v_pair(i, j, dp, 2)])
                    
        def valid_cells(cells):
            for i, xi in enumerate(cells):
                for j, xj in enumerate(cells):
                    if i < j:
                        for d in range(1, self.ksq+1):
                            claus.append([-self.v_pair(xi[0], xi[1], d, 1), -self.v_pair(xj[0], xj[1], d ,1)])
                            claus.append([-self.v_pair(xi[0], xi[1], d, 2), -self.v_pair(xj[0], xj[1], d, 2)])
        for i in range(1, self.ksq+1): #each row and col should not have any number more than once
            valid_cells([(i, j) for j in range(1, self.ksq+1)])
            valid_cells([(j, i) for j in range(1, self.ksq+1)])
        for d in range(1, self.ksq + 1):
            for i in range(1, self.ksq + 1):
                claus.append([self.v_pair(i, j, d, 1) for j in range(1, self.ksq + 1)])
                claus.append([self.v_pair(i, j, d, 2) for j in range(1, self.ksq + 1)])
                claus.append([self.v_pair(j, i, d, 1) for j in range(1, self.ksq + 1)])
                claus.append([self.v_pair(j, i, d, 2) for j in range(1, self.ksq + 1)])
                
        blocks = [(i*self.k+1) for i in range(0,self.k)] #no repetition in k x k grid 
        for i in blocks:
            for j in blocks:
                valid_cells([(i+m%self.k, j+m//self.k) for m in range(0,self.ksq)])
        for i in range(1, self.ksq + 1):
            for j in range(1, self.ksq + 1):
                for d in range(1, self.ksq + 1):
                    claus.append([-self.v_pair(i, j, d, 1), -self.v_pair(i, j, d, 2)])
        
        return claus

    

    def solve_sudoku_pair(self, sudoku1, sudoku2):
        clauses = CNF()
        cl1 = self.make_clause_pair()
        for i in range(1, self.ksq+1):
            for j in range(1, self.ksq+ 1):
                if sudoku1[i-1][j-1] != 0:
                    clauses.append([self.v_pair(i, j , sudoku1[i-1][j-1], 1)])
                if sudoku2[i-1][j-1] != 0:
                    clauses.append([self.v_pair(i, j, sudoku2[i-1][j-1], 2)])
        
        solution = Solver(name='m22', bootstrap_with=clauses)
        solution.append_formula(cl1)
        solution.solve()
        model = solution.get_model()
        print(model)
        return model

    def print_sudoku(self, sudoku1, sudoku2):
        print("Sudoku1:")
        for i in range(0, self.ksq):
            for j in range(0, self.ksq):
                print(sudoku1[i][j], end=", ")
            print()
            
        print("Sudoku2:")
        for i in range(0, self.ksq):
            for j in range(0, self.ksq):
                print(sudoku2[i][j], end=", ")
            print()
       

    def solve_driver(self):
        (sudoku1, sudoku2) = self.read_sudoku_pair()
        model = self.solve_sudoku_pair(sudoku1, sudoku2)
        if model == None:
            print("No solution exists")
            return
        for i in range(1, self.ksq+1):
            for j in range(1, self.ksq+1):
                for d in range(1, self.ksq+ 1):
                    if self.v_pair(i, j ,d, 1) in model:
                        sudoku1[i-1][j-1] = d
                    if self.v_pair(i, j ,d, 2) in model:
                        sudoku2[i-1][j-1] = d
        self.print_sudoku(sudoku1, sudoku2)
if __name__ == "__main__":
    print("Input k : ")
    k = (int)(input())
    
    s = Sudoku_Solver(k)
    s.solve_driver()
    
