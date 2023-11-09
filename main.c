#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include "mtrxop.h"

#define NULLPATH -1

int mtrxOrder, submtrxOrder, submtrxSize;

void display_help() {
    printf("Usage: <program_name> [options] <input_file>\n");
    printf("    -o --output   Specify custom output name\n");
    printf("    -h --help     Display this help message\n");
}


struct option long_options[] = {
    {"output", required_argument, 0, 'o'},
    {"help", no_argument, 0, 'h'},
    {0, 0, 0, 0}
};


////////////////////
int main(int argc, char** argv) {
  int rank, nprocs;
  double start, end;
  char* input;
  char* output = NULL;
  FILE* inputFile;

  MPI_Init(&argc, &argv);

  start = MPI_Wtime();

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int opt;
  while ((opt = getopt_long(argc, argv, "ho:r:", long_options, NULL)) != -1) {
    switch (opt) {
      case 'o':
        output = optarg;
        break;
      case 'h': 
        if (rank == 0)
          display_help();
        MPI_Finalize();
        return 0;
      default:
        if (rank == 0)
          display_help();
        MPI_Finalize();
        return 0;
    }
  }
  GRID_TYPE grid;
  setupGrid(&grid);

  if (optind >= argc) {
      fprintf(stderr, "Missing input file. Use --help for help.\n");
      MPI_Finalize();
      return 1;
  }

  input = argv[optind];
  inputFile = fopen(input, "r");

  if (inputFile == NULL) {
    if (rank == 0) {
      fprintf(stderr, "Error: Unable to open file %s\n", input);
    }
    MPI_Finalize();
    return 1;
  }
  if (grid.my_rank == 0) {
    fscanf(inputFile, "%d", &mtrxOrder);
  }

  MPI_Bcast(&mtrxOrder, 1, MPI_INT, 0, grid.comm);

  submtrxOrder = (int)sqrt(nprocs);
  submtrxSize = mtrxOrder / submtrxOrder;

  if (nprocs != submtrxOrder * submtrxOrder) {
    MPI_Finalize();
    if (rank == 0) {
      printf("Error: The number of processes P is not a perfect square.\n");
    }
    exit(0);
  }

  if (mtrxOrder % submtrxSize != 0) {
    MPI_Finalize();
    if (rank == 0) {
      printf("Error: The dimension of the matrix N is not divisible by q.\n");
    }
    exit(0);
  }

  int totalMatrix[mtrxOrder * mtrxOrder];

  if (grid.my_rank == 0) {
    int rank_dest;
    int counter[nprocs];
    zeroArray(&counter[0], nprocs);
    // scan matrix
    for (int i = 0; i < mtrxOrder; i++) {
      for (int j = 0; j < mtrxOrder; j++) {
        rank_dest = (i / submtrxSize) * submtrxOrder + (j / submtrxSize);
        fscanf(inputFile, "%d", &totalMatrix[rank_dest * submtrxSize * submtrxSize + counter[rank_dest]]);

        if (totalMatrix[rank_dest * submtrxSize * submtrxSize + counter[rank_dest]] == 0 && i != j)
          totalMatrix[rank_dest * submtrxSize * submtrxSize + counter[rank_dest]] = NULLPATH;
        counter[rank_dest] += 1;
      }
    }
  }

  int** matrixA = initMatrix(submtrxSize);
  MPI_Scatter(&totalMatrix[0], submtrxSize * submtrxSize, MPI_INT, &matrixA[0][0], submtrxSize * submtrxSize,
              MPI_INT, 0, grid.comm);

  int g = 1;
  int** matrixB = initMatrix(submtrxSize);
  int** matrixC = initMatrix(submtrxSize);
  int** tempMatrix = initMatrix(submtrxSize);
  copyMatrix(matrixA, matrixB, submtrxSize);
  copyMatrix(matrixA, matrixC, submtrxSize);
  while (g < mtrxOrder) {
    Fox(&grid, &matrixA[0], &matrixB[0], &matrixC[0], tempMatrix, submtrxSize); 
    copyMatrix(&matrixC[0], &matrixA[0], submtrxSize);
    copyMatrix(&matrixC[0], &matrixB[0], submtrxSize);
    g *= 2;
  }

  if (grid.my_rank == 0)
    zeroArray(&totalMatrix[0], mtrxOrder * mtrxOrder);

  MPI_Gather(matrixC[0], submtrxSize * submtrxSize, MPI_INT, &totalMatrix[0], submtrxSize * submtrxSize,
             MPI_INT, 0, MPI_COMM_WORLD);

  if (grid.my_rank == 0) {
    int rank_dest;
    int counter[nprocs];

    FILE* outputFile = fopen(output, "w");

    zeroArray(&counter[0], nprocs);
    for (int i = 0; i < mtrxOrder; i++) {
      for (int j = 0; j < mtrxOrder; j++) {
        rank_dest = (i / submtrxSize) * submtrxOrder + (j / submtrxSize);
        if (totalMatrix[rank_dest * submtrxSize * submtrxSize + counter[rank_dest]] == NULLPATH) {
          if (output == NULL) 
            printf("%d%c", 0, j == mtrxOrder - 1 ? '\n' : ' ');
          else
            fprintf(outputFile, "%d%c", 0, j == mtrxOrder - 1 ? '\n' : ' ');
        } else {
          if (output == NULL)
            printf("%d%c", totalMatrix[rank_dest * submtrxSize * submtrxSize + counter[rank_dest]],
                   j == mtrxOrder - 1 ? '\n' : ' ');
          else
            fprintf(outputFile, "%d%c", totalMatrix[rank_dest * submtrxSize * submtrxSize + counter[rank_dest]],
                   j == mtrxOrder - 1 ? '\n' : ' ');
        }
        counter[rank_dest] += 1;
      }
    }

    if (output != NULL) {
      fprintf(outputFile, "\n");
      fclose(outputFile);
    } else 
      printf("\n");
    
  }

  fclose(inputFile);

  free(matrixA[0]);
  free(matrixA);
  free(matrixB[0]);
  free(matrixB);
  free(matrixC[0]);
  free(matrixC);
  free(tempMatrix[0]);
  free(tempMatrix);


  if (rank == 0) {
    end = MPI_Wtime();
    double elapsed = end - start;
    printf("Result returned in process %i in %f seconds\n", rank, elapsed);
  }

  MPI_Finalize();
}
