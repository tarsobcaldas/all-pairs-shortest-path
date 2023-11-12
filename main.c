#include <openmpi-x86_64/mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include "mtrxop.h"

#define NULLPATH -1

int mtrxSize, submtrxOrder, submtrxSize;

void display_help() {
    printf("Usage: <program_name> [options] <input_file>\n");
    printf("    -i <file> --input <file>    Set matrix input file");
    printf("    -o <file> --output <file>   Specify custom output name\n");
    printf("    -h --help                   Display this help message\n");
    printf("    -t --timing-only            Don't print the solution, only the timing\n");
    printf("    -r --random-matrix <size>   Use random matrix as input\n");
    printf("                                (only if no input file provided)\n");
}


struct option long_options[] = {
    {"output", required_argument, 0, 'o'},
    {"help", no_argument, 0, 'h'},
    {"timing-only", no_argument, 0, 't'},
    {0, 0, 0, 0}
};


int main(int argc, char** argv) {
  int rank, nprocs, randomMtrx;
  double start, end;
  char* input;
  char* output = NULL;
  FILE* inputFile;
  randomMtrx = 0;

  MPI_Init(&argc, &argv);

  start = MPI_Wtime();

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int opt;
  while ((opt = getopt_long(argc, argv, "hi:o:tr:", long_options, NULL)) != -1) {
    switch (opt) {
      case 'i':
        input = optarg;
        break;
      case 'o':
        output = optarg;
        break;
      case 'r':
        randomMtrx = 1;
        mtrxSize = atoi(optarg);
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

  if (input == NULL && randomMtrx == 0) {
      fprintf(stderr, "Missing input file. Use --help for help.\n");
      MPI_Finalize();
      return 1;
  } else if (input != NULL) {
    if (randomMtrx == 1) {
      printf(stderr, "Input file chosen, ignoring random matrix option");
      randomMtrx = 0;
    }

    inputFile = fopen(input, "r");
    if (inputFile == NULL) {
      if (rank == 0) {
        fprintf(stderr, "Error: Unable to open file %s\n", input);
      }
      MPI_Finalize();
      return 1;
    }
    if (grid.my_rank == 0) {
      fscanf(inputFile, "%d", &mtrxSize);
    }
  } 

  MPI_Bcast(&mtrxSize, 1, MPI_INT, 0, grid.comm);

  submtrxOrder = (int)sqrt(nprocs);
  submtrxSize = mtrxSize / submtrxOrder;

  if (nprocs != submtrxOrder * submtrxOrder) {
    if (rank == 0) {
      fprintf(stderr,"Error: The number of processes P is not a perfect square.\n");
    }
    MPI_Finalize();
    exit(0);
  }

  if (mtrxSize % submtrxSize != 0) {
    if (rank == 0) {
      fprintf(stderr,"Error: The dimension of the matrix N is not divisible by q.\n");
    }
    MPI_Finalize();
    exit(0);
  }

  int totalMatrix[mtrxSize * mtrxSize];

  if (grid.my_rank == 0) {
    int rank_dest;
    int counter[nprocs];
    zeroArray(&counter[0], nprocs);
    // scan matrix
    for (int i = 0; i < mtrxSize; i++) {
      for (int j = 0; j < mtrxSize; j++) {
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
  while (g < mtrxSize) {
    Fox(&grid, &matrixA[0], &matrixB[0], &matrixC[0], tempMatrix, submtrxSize); 
    copyMatrix(&matrixC[0], &matrixA[0], submtrxSize);
    copyMatrix(&matrixC[0], &matrixB[0], submtrxSize);
    g *= 2;
  }

  if (grid.my_rank == 0)
    zeroArray(&totalMatrix[0], mtrxSize * mtrxSize);

  MPI_Gather(matrixC[0], submtrxSize * submtrxSize, MPI_INT, &totalMatrix[0], submtrxSize * submtrxSize,
             MPI_INT, 0, MPI_COMM_WORLD);

  if (grid.my_rank == 0) {
    int rank_dest;
    int counter[nprocs];

    FILE* outputFile = fopen(output, "w");

    zeroArray(&counter[0], nprocs);
    for (int i = 0; i < mtrxSize; i++) {
      for (int j = 0; j < mtrxSize; j++) {
        rank_dest = (i / submtrxSize) * submtrxOrder + (j / submtrxSize);
        if (totalMatrix[rank_dest * submtrxSize * submtrxSize + counter[rank_dest]] == NULLPATH) {
          if (output == NULL) 
            printf("%d%c", 0, j == mtrxSize - 1 ? '\n' : ' ');
          else
            fprintf(outputFile, "%d%c", 0, j == mtrxSize - 1 ? '\n' : ' ');
        } else {
          if (output == NULL)
            printf("%d%c", totalMatrix[rank_dest * submtrxSize * submtrxSize + counter[rank_dest]],
                   j == mtrxSize - 1 ? '\n' : ' ');
          else
            fprintf(outputFile, "%d%c", totalMatrix[rank_dest * submtrxSize * submtrxSize + counter[rank_dest]],
                   j == mtrxSize - 1 ? '\n' : ' ');
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
    printf("%f\n", elapsed);
  }

  MPI_Finalize();
}
