#ifndef MTRXOP_H
#define MTRXOP_H

#include <mpi.h>

typedef struct {
  int nprocs;       
  MPI_Comm comm;    
  MPI_Comm row_comm; 
  MPI_Comm col_comm;
  int gridOrder;
  int row;     
  int col;    
  int rank;  
} GRID_TYPE;


void matrixMultiply(int** matrixA, int** matrixB, int** matrixC, int size);

void Fox(GRID_TYPE* grid, int** matrixA, int** matrixB, int** matrixC, int size);

void setupGrid(GRID_TYPE *grid);

int** initMatrix(int length);

void zeroArray(int *matrix, int length);

void zeroMatrix(int** matrix, int length);

void printMatrix(int** matrix, int length);

void copyMatrix(int** matrixA, int** matrixB, int length);

void generateRandomArray(int array[], int length);

#endif
