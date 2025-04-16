On this file wil explain how to convert sudoku to a combinatorial optimization problem and use the combinatorial optimization algorithm to solve sudoku.
Note: im not too good explaining but you can read this to have a base and then study the Python version of the algorithm.

First thing you need to know is what is sudoku and their rules:
Sudoku is a logical puzzle that needs you to fill all cells and then checking if all rows, columns and blocks doesn't contain a repetitive number.

Sudokus can have sizes likes 4x4, 9x9, 16x16 where the size is equals to N^2, like 2^2 = 4, 3^2 = 9, 4^2 = 16

Sudokus rules are very simple.

The second step to conversion is defining what gonna be the global minimum, let me explain how to do this using a 9x9 Sudoku:
We have 9 row with 9 numbers, a total of 81 unique numbers on each row
We have 9 columns with 9 numbers, a total of 81 unique numbers on each column
we have 9 blocks with 9 numbers, a total of 81 unique numbers on each block

the global minimum cost gonna be 81 + 81 + 81 = 243 or a more general formula being: N ^ 2 * 3

We can represent rows, columns, blocks has subgraphs doing that with an algorithm, check "solver.py" file to see this algorithm

After that we need to create a algorithm to calculate the global cost following those rules the idea is that if a number repeats on some row, column or block we gonna
make the repetitive number have a cost 2 instead of 1, check the "solver.py" file again to see this algorithm

The second step is done and now comes the main algorithm:
Create a greedy tour by putting unique numbers based on a subgraph like rows, columns or blocks but still gonna be unoptimal, apply a two-opt reverse that instead of 
decreasing the cost gonna increase, this is for the initial tour because after worsing the cost of initial tour you usually gets better results according to my tests. 
and then combines the two opt with swap algorithm to pertubate the tour making the tour variates and exploring more possibilites.

all instances are from:
https://www.sudokuwiki.org/Weekly-Sudoku.aspx

Those are "unsolvable instances" and sudokuwiki solver can't solve those instances but my algorithm solved all 3 instances i tested, you can add more instances but
you need to do a manual input because the site doesn't have a downloading option or at least i didn't see any.