## HPC project

HPC project, working on the Cineca HPC Infrastructure Galileo100.

**Description** 

Given a square matrix *A[N]x[N]*, containing N^2 real values belonging to the interval [0, 1]. We define the neighborhood of *A[i][j]* the submatrix of A of size 3x3 which has *A[i][j]* in the central position.

We want to calculate a matrix of integers *B[N]x[N]* obtained in this way: the value of each element *B[i][j]* is determined based on the value of the arithmetic mean *M_ij* of the values of the elements belonging to the neighborhood of *A[i][j]*, according to the following rule:
- if *M_ij* < 0,5 →  *B[i][j]* = 0 
- if *M_ij* ≥ 0,5 →  *B[i][j]* = 1 
