NP-complete problems has combinatorial optimization problems
By Pedro Guilherme Cabral da Silva

This directory contains 3 solvers that shares the same idea, inspired by GRASP and LKH.
Since TSP and HCP are very similar, it is easy to make a reduction from HCP to TSP and theres no need to have a readmfile here because it's very similar to LKH.
For Sudoku, I transformed it into a combinatorial optimization problem. For more information, see the Sudoku readme file.

The main idea of the algorithm is divided into 3 concepts:
Initial tour generation with greedy tour and Two-opt reverse (improves the initial tour according to my tests).
Two-opt
Swap (deterministic perturbation)

You only need to know these 3 things and you will be able to create efficient solvers for almost any NP-complete and NP-hard problem, transforming it into a 
combinatorial optimization problem and solving it with this metaheuristic that outperforms known heuristics and metaheuristics, such as genetic algorithm, 
ant colony optimization, GRASP, simulated annealing and others, at the cost of higher time complexity but still have a average polynomial runtime of O(N^5) and worst time complexity of O(N^6).

The name of algorithm is "combinatorial optimization algorithm" and may be better than constraint satisfaction because it can be used to resolve the following problems:
Is there a value less than x?
Is there a value bigger than x?
Is there a value equals to x?
What is the minimum value?
What is the maximum value?

while constraint satisfaction can only answers decisions problems like:
Is there a value less than x?
Is there a value bigger than x?
Is there a value equals to x?

The algorithm can't beat all instances of all NP-complete problems, it fails on hamiltonian graphs from the FHCP challenge set but the perfomance on others problems like "Weekly Unsolvable Sudoku" and Travelling Salesman Problem is really good.

I also want to share some things i learned while studying NP-complete problems:
First thing is that we can have easy, medium, hard and no-solution instances and if it's hard to us, it's hard for computer too so it's computation expensive to 
find the solution for hard instances. For example easy instances simple heuristics can easily find the solution, for medium instances you need good meta-heuristics 
to find the solution and for hard instances you probably need exact solvers with exponential worst time complexity to find the solution unless the heuristic is really 
good. If the instance doesn't have a solution you probably can't proof that this instance doesn't have a solution with a exact algorithm with exponential worst time 
complexity because it's gonna takes years to test all possibilites even pruning the binary tree.

Second thing is that some NP-complete problems rely on luck to solve, for example on minesweep you can get stuck on some ambiguous cases where you doesn't have enough clues to decide where the mines are.

Planning to create new solvers for knapsack problem and Rubik's cube.

Donate if you want:
bitcoin:bc1ql7h7nlzewj7g6jaj9qv3x90mtwggudyjyx3m57hewh5cgw572fys4rwdpj
