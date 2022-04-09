from pysat.formula import CNF
from pysat.solvers import Solver
from pprint import pprint
import random
from time import time
import csv
class Sudoku_Solver:
    def __init__(self, k):
        self.k = k
        self.ksq = k**2
    def v(self, i, j, d):
        return (self.ksq**2)*(i-1) + self.ksq*(j-1) + d
    def v_pair(self, i, j, d, m): #m = 1 or 2
        return 2*((self.ksq**2)*(i-1) + self.ksq*(j-1) + d) + m

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
        for d in range(1, self.ksq + 1):
            for i in range(1, self.ksq + 1):
                claus.append([self.v_pair(i, j, d, 1) for j in range(1, self.ksq + 1)])
                claus.append([self.v_pair(i, j, d, 2) for j in range(1, self.ksq + 1)])
                claus.append([self.v_pair(j, i, d, 1) for j in range(1, self.ksq + 1)])
                claus.append([self.v_pair(j, i, d, 2) for j in range(1, self.ksq + 1)])
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

        blocks = [(i*self.k+1) for i in range(0,self.k)] #no repetition in k x k grid 
        for i in blocks:
            for j in blocks:
                valid_cells([(i+m%self.k, j+m//self.k) for m in range(0,self.ksq)])

        for i in range(1, self.ksq + 1):
            for j in range(1, self.ksq + 1):
                for d in range(1, self.ksq + 1):
                    claus.append([-self.v_pair(i, j, d, 1), -self.v_pair(i, j, d, 2)])
        

        return claus

    def make_clause_single(self):
        claus=[]
        for i in range(1, self.ksq+1):
            for j in range(1, self.ksq+1):
                claus.append([self.v(i, j ,d) for d in range(1, self.ksq+1)]) #each box should have atleast one number 
                for d in range(1, self.ksq):
                    for dp in range(d+1, self.ksq+1):
                        claus.append([-self.v(i, j, d), -self.v(i, j, dp)]) # each box should have only one number assigned
        for d in range(1, self.ksq + 1):
            for i in range(1, self.ksq + 1):
                claus.append([self.v(i, j, d) for j in range(1, self.ksq + 1)])
                claus.append([self.v(j, i, d) for j in range(1, self.ksq + 1)])

        def valid_cells(cells):
            for i, xi in enumerate(cells):
                for j, xj in enumerate(cells):
                    if i < j:
                        for d in range(1, self.ksq+1):
                            claus.append([-self.v(xi[0], xi[1], d), -self.v(xj[0], xj[1], d)])
        
        for i in range(1, self.ksq+1): #each row and col should not have any number more than once
            valid_cells([(i, j) for j in range(1, self.ksq+1)])
            valid_cells([(j, i) for j in range(1, self.ksq+1)])

        blocks = [(i*self.k+1) for i in range(0,self.k)] #no repetition in k x k grid 
        for i in blocks:
            for j in blocks:
                valid_cells([(i+m%self.k, j+m//self.k) for m in range(0,self.ksq)])

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
        return model
    def pair_solution_checker(self, sudoku1, sudoku2, add_claus):
        clauses = CNF()
        for i in range(1, self.ksq+1):
            for j in range(1, self.ksq+ 1):
                if sudoku1[i-1][j-1] != 0:
                    clauses.append([self.v_pair(i, j , sudoku1[i-1][j-1], 1)])
                if sudoku2[i-1][j-1] != 0:
                    clauses.append([self.v_pair(i, j, sudoku2[i-1][j-1], 2)])
        solution = Solver(name='m22', bootstrap_with=clauses)
        solution.append_formula(add_claus)
        exists = solution.solve()
        return exists
        
    def solve_sudoku_single(self, sudoku):
        clause = CNF()
        cl1 = self.make_clause_single()
        for i in range(1, self.ksq+1):
            for j in range(1, self.ksq+ 1):
                if sudoku[i-1][j-1] != 0:
                    clause.append([self.v(i, j , sudoku[i-1][j-1])])
        sol = Solver('m22', bootstrap_with=clause)
        sol.append_formula(cl1)
        e = sol.solve()
        if e == 0:
            print("NO solution exists")
            return None
        model = sol.get_model()
        for i in range(1, self.ksq+ 1):
            for j in range(1, self.ksq+ 1):
                for d in range(1, self.ksq + 1):
                    if self.v(i, j, d) in model:
                        sudoku[i-1][j-1] = d
        return sudoku
class Generator:
    def __init__(self, k):
        self.k = k
        self.ksq = k**2

    def v(self, i, j ,d):
        return (self.ksq**2)*(i-1) + self.ksq*(j-1) + d

    def v_pair(self, i, j, d, m): #m = 1 or 2
        return 2*((self.ksq**2)*(i-1) + self.ksq*(j-1) + d) + m
    
    def make_negation_clause(self, grid1, grid2):
        c = []
        for i in range(1, self.ksq + 1):
            for j in range(1, self.ksq +1):
                c.append(-self.v_pair(i, j, grid1[i-1][j-1], 1))
                c.append(-self.v_pair(i, j, grid2[i-1][j-1], 2))
        
        return c
    
    def generate_single_random(self):
        puzzle = []
        for i in range(0, self.ksq):
            row = [0]*self.ksq
            puzzle.append(row)
        puzzle[0][0] = random.randint(1, self.ksq)
        puzzle[self.ksq-1][self.ksq - 1] = random.randint(1, self.ksq)
        return puzzle
    def generate_second_sudoku(self, grid1):
        s = Sudoku_Solver(self.k)
        grid2 = []
        for i  in range(0, self.ksq):
            row = [0]*self.ksq
            grid2.append(row)
        
        m2 =  s.solve_sudoku_pair(grid1, grid2)
        for i in range(1, self.ksq+ 1):
            for j in range(1, self.ksq + 1):
                for d in range(1, self.ksq + 1):
                    if self.v_pair(i, j , d, 2) in m2:
                        grid2[i-1][j-1] = d
        
        return grid2
    
    def maximal_sudoku_gen(self):
        sol = Sudoku_Solver(self.k)
        s1 = self.generate_single_random()
        sudoku1 = sol.solve_sudoku_single(s1)
        sudoku2 = self.generate_second_sudoku(sudoku1)
        filename  = str(self.k) + "_full.csv"
        with open(filename, 'w') as f:
            csvwriter = csv.writer(f)
            csvwriter.writerows(sudoku1)
            csvwriter.writerows(sudoku2)
            f.close()
        neg_claus = self.make_negation_clause(sudoku1, sudoku2)
        pair_clause = sol.make_clause_pair()
        add_claus = [neg_claus]
        for c in pair_clause:
            add_claus.append(c)
        ind = random.sample(range(0, 2*(self.ksq**2)), 2*(self.ksq**2))
        for i in range(0, 2*(self.ksq**2)):
            if(ind[i] > self.ksq**2 -1):
                ind[i] -= self.ksq**2
                x = ind[i]//self.ksq
                y = ind[i]%self.ksq
                tmp = sudoku2[x][y]
                sudoku2[x][y]= 0
                e = sol.pair_solution_checker(sudoku1, sudoku2, add_claus)
                if e:
                    sudoku2[x][y] = tmp
            else:
                x = ind[i]//self.ksq
                y = ind[i]%self.ksq
                tmp = sudoku1[x][y]
                sudoku1[x][y]= 0
                e = sol.pair_solution_checker(sudoku1, sudoku2, add_claus)
                if e:
                    sudoku1[x][y] = tmp

        return (sudoku1, sudoku2)
    
    def print_sudoku(self, sudoku1, sudoku2):
        print("Sudoku1:")
        pprint(sudoku1)
        print("Sudoku2:")
        pprint(sudoku2)
        filename  = str(self.k) + "_empty.csv"
        with open(filename, 'w') as f:
            csvwriter = csv.writer(f)
            csvwriter.writerows(sudoku1)
            csvwriter.writerows(sudoku2)
            f.close()
    
    def generate_driver(self):    
        (sudoku1, sudoku2) = self.maximal_sudoku_gen()
        self.print_sudoku(sudoku1, sudoku2)
    
if __name__=="__main__":
    print("Input k : ")
    k = int(input())
    gen = Generator(k)
    gen.generate_driver()
    