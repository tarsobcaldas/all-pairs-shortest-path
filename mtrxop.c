#include "mtrxop.h"
#include <math.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NULLPATH -1

// Allocate memory for the matrix
int** initMatrix(int length) {
  int* m = (int*)malloc(length * length * sizeof(int));

  int** M = (int**)malloc(length * sizeof(int *));

  for (int i = 0; i < length; i++)
    M[i] = &(m[i * length]);

  return M;
}

// Make all elements of array equal to 0
void zeroArray(int *matrix, int length) {
  for (int i = 0; i < length; i++)
    matrix[i] = 0;
}

// Make all elements of matrix equal to 0
void zeroMatrix(int **matrix, int length) {
  for (int i = 0; i < length; i++) {
    for (int j = 0; j < length; j++) {
      matrix[i][j] = 0;
    }
  }
}

void printMatrix(int** matrix, int length) {
  for (int i = 0; i < length; i++) {
    for (int j = 0; j < length; j++) {
      printf("%d ", matrix[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

void copyMatrix(int** matrixA, int** matrixB, int length) {
  for (int i = 0; i < length; i++) {
    for (int j = 0; j < length; j++)
      matrixB[i][j] = matrixA[i][j];
  }
}

void matrixMultiply(int** matrixA, int** matrixB, int** matrixC, int size) {
  int i, j, k, min;
  for (i = 0; i < size; i++) {
    for (j = 0; j < size; j++) {
      min = NULLPATH;
      for (k = 0; k < size; k++) {
        if (matrixA[i][k] > 0 && matrixB[k][j] > 0) {
          if ((min < 0) || (matrixA[i][k] + matrixB[k][j]) < min) 
            min = (matrixA[i][k] + matrixB[k][j]);
        } 
        if ((min >= 0) && (matrixC[i][j] < 0 || matrixC[i][j] > min))
          matrixC[i][j] = min;
      }
    }
  }
}


void setupGrid(GRID_TYPE* grid) {
  int rank, dimensions[2], periods[2], coordinates[2], varying_coords[2];

  MPI_Comm_size(MPI_COMM_WORLD, &(grid->nprocs));
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  grid->submtrxOrder = (int)sqrt(grid->nprocs);
  dimensions[0] = dimensions[1] = grid->submtrxOrder;
  periods[0] = periods[1] = 1;
  MPI_Cart_create(MPI_COMM_WORLD, 2, dimensions, periods, 1, &(grid->comm));
  MPI_Comm_rank(grid->comm, &(grid->my_rank));
  MPI_Cart_coords(grid->comm, grid->my_rank, 2, coordinates);
  grid->my_row = coordinates[0];
  grid->my_col = coordinates[1];

  varying_coords[0] = 0;
  varying_coords[1] = 1;
  MPI_Cart_sub(grid->comm, varying_coords, &(grid->row_comm));
  varying_coords[0] = 1;
  varying_coords[1] = 0;
  MPI_Cart_sub(grid->comm, varying_coords, &(grid->col_comm));
}


void Fox(GRID_TYPE* grid, int** matrixA, int** matrixB, int** matrixC,
         int** tempMatrix, int size) {
  int bcast_root, source, dest;
  int tag = 0;

  source = (grid->my_row + 1) % grid->submtrxOrder;
  dest = (grid->my_row + grid->submtrxOrder - 1) %
         grid->submtrxOrder; 

  int step;
  for (step = 0; step < grid->submtrxOrder; step++) {
    bcast_root = (grid->my_row + step) % grid->submtrxOrder;
    if (bcast_root == grid->my_col) {
      MPI_Bcast(matrixA[0], size * size, MPI_INT, bcast_root, grid->row_comm);
      matrixMultiply(matrixA, matrixB, matrixC, size);
    } else {
      MPI_Bcast(tempMatrix[0], size * size, MPI_INT, bcast_root,
                grid->row_comm); // &tempA[0][0] redundante
      matrixMultiply(tempMatrix, matrixB, matrixC, size);
    }
    MPI_Sendrecv_replace(matrixB[0], size * size, MPI_INT, dest, tag, source,
                         tag, grid->col_comm, MPI_STATUS_IGNORE);
  }
}

