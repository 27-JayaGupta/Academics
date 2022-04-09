# Intructions to run the program

## Solver

<ul>
<li>On running sudoku.py , input k value(3 or 4 or 5). The solver will automatically pick the unsolved sudokus from the tests folder(3_empty.csv or 4_empty.csv or 5_empty.csv). The corresponding completely filled sudokus can be find in the files in tests folder(k_full.csv).</li>

<li>Inorder to run sudokus which have no solution (in file tests/3_nosolution and 4_nosolution) , comment Line 17 in sudoku.py and uncomment Line 18 or 19 as per need.</li>
</ul>

## Generator

<ul>
<li>To run sudoku generator of dimension(k^2 * k^2) satisfying the given constraints , just run generator.py and give k value as input. The generator will output an empty sudoku pair.</li>
<li>One limitation of the generator is that for k values 5 and beyond , it takes considerable time and hardware resources to run , thats why is not feasible to run on local machines.
</ul>
