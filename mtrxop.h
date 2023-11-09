#ifndef MTRXOP_H
#define MTRXOP_H

#include <mpi.h>

typedef struct {
  int nprocs;             // total number of processes
  MPI_Comm comm;     // communicator for entire grid
  MPI_Comm row_comm; // communicator for my row
  MPI_Comm col_comm; // communicator for my col
  int submtrxOrder;    // order of grid
  int my_row;        // my row number
  int my_col;        // my column number
  int my_rank;       // my rank in the grid communicator
} GRID_TYPE;


void matrixMultiply(int** matrixA, int** matrixB, int** matrixC, int size);

void Fox(GRID_TYPE* grid, int** matrixA, int** matrixB, int** matrixC, int** tempMatrix, int size);

void setupGrid(GRID_TYPE *grid);

int** initMatrix(int length);

void zeroArray(int *matrix, int length);

void zeroMatrix(int** matrix, int length);

void printMatrix(int** matrix, int length);

void copyMatrix(int** matrixA, int** matrixB, int length);

void randomizeMatrix(int** matrix, int length);

#endif
