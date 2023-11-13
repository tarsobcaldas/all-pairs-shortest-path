#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <getopt.h>

#include "mtrxop.h"

#define NULLPATH -1

int mtrxSize, submtrxOrder, submtrxSize;

void display_help() {
    printf("Usage: mpirun <processes> shortest-path [options]\n");
    printf("    -i <file> --input <file>    Set matrix input file\n");
    printf("    -o <file> --output <file>   Specify custom output name\n");
    printf("    -h --help                   Display this help message\n");
    printf("    -t --timing-only            Don't print the solution, only the timing\n");
    printf("    -r --random-matrix <size>   Use random matrix as input\n");
    printf("                                (only if no input file provided)\n");
}


struct option long_options[] = {
    {"input", required_argument, 0, 'i'},
    {"output", required_argument, 0, 'o'},
    {"help", no_argument, 0, 'h'},
    {"timing-only", no_argument, 0, 't'},
    {"random-matrix", required_argument, 0, 'r'},
    {0, 0, 0, 0}
};


int main(int argc, char** argv) {
  int rank, nprocs, randomMtrx;
  double start, end;
  char* inputFile = NULL;
  char* outputFile = NULL;
  FILE* input;
  int printResult = 1;

  randomMtrx = 0;
  MPI_Init(&argc, &argv);

  start = MPI_Wtime();

  MPI_Comm_size(MPI_COMM_WORLD, &nprocs);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int opt;
  while ((opt = getopt_long(argc, argv, "i:o:htr:", long_options, NULL)) != -1) {
    switch (opt) {
      case 'i':
        inputFile = optarg;
        break;
      case 'o':
        outputFile = optarg;
        break;
      case 'r':
        randomMtrx = 1;
        mtrxSize = atoi(optarg);
        break;
      case 'h': 
        if (rank == 0)
          display_help();
        MPI_Finalize();
        return 0;
      case 't':
        printResult = 0;
        break;
      default:
        if (rank == 0)
          display_help();
        MPI_Finalize();
        return 0;
    }
  }
  GRID_TYPE grid;
  setupGrid(&grid);

  if (inputFile == NULL) {
    if (randomMtrx == 0) {
      if (grid.rank == 0) 
        fprintf(stderr, "Missing input file. Use --help for help.\n");
      MPI_Finalize();
      return 1;
    }
  } else if (inputFile != NULL) {
    if (randomMtrx == 1) {
      if (grid.rank == 0)
        fprintf(stderr, "Input file chosen, ignoring random matrix option\n");
      randomMtrx = 0;
    }

    input = fopen(inputFile, "r");
    if (input == NULL) {
      if (grid.rank == 0)
        fprintf(stderr, "Error: Unable to open file %s\n", inputFile);
      MPI_Finalize();
      return 1;
    }
    fscanf(input, "%d", &mtrxSize);
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

  if (randomMtrx) {
    generateRandomArray(totalMatrix, mtrxSize);
  }

  if (grid.rank == 0) {
    int rank_dest;
    int counter[nprocs];
    zeroArray(&counter[0], nprocs);
    // scan matrix
    for (int i = 0; i < mtrxSize; i++) {
      for (int j = 0; j < mtrxSize; j++) {
        rank_dest = (i / submtrxSize) * submtrxOrder + (j / submtrxSize);
        if (!randomMtrx)
          fscanf(input, "%d", &totalMatrix[rank_dest * submtrxSize * submtrxSize + counter[rank_dest]]);

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
  copyMatrix(matrixA, matrixB, submtrxSize);
  copyMatrix(matrixA, matrixC, submtrxSize);
  while (g < mtrxSize) {
    Fox(&grid, &matrixA[0], &matrixB[0], &matrixC[0], submtrxSize); 
    copyMatrix(&matrixC[0], &matrixA[0], submtrxSize);
    copyMatrix(&matrixC[0], &matrixB[0], submtrxSize);
    g *= 2;
  }

  if (grid.rank == 0)
    zeroArray(&totalMatrix[0], mtrxSize * mtrxSize);

  MPI_Gather(matrixC[0], submtrxSize * submtrxSize, MPI_INT, &totalMatrix[0], submtrxSize * submtrxSize,
             MPI_INT, 0, MPI_COMM_WORLD);

  if (grid.rank == 0) {
    int rankDest;
    int counter[nprocs];
    FILE* output;

    if (outputFile != NULL)
      output = fopen(outputFile, "w");
    else 
      output = stdout;

    zeroArray(&counter[0], nprocs);

    if (printResult)
      for (int i = 0; i < mtrxSize; i++) {
        for (int j = 0; j < mtrxSize; j++) {
          rankDest = (i / submtrxSize) * submtrxOrder + (j / submtrxSize);
          if (totalMatrix[rankDest * submtrxSize * submtrxSize + counter[rankDest]] == NULLPATH) {
            fprintf(output, "%d%c", 0, j == mtrxSize - 1 ? '\n' : ' ');
          } else {
            fprintf(output, "%d%c", totalMatrix[rankDest * submtrxSize * submtrxSize + counter[rankDest]],
                   j == mtrxSize - 1 ? '\n' : ' ');
          }
          counter[rankDest] += 1;
        }
      }

    if (outputFile != NULL) {
      fprintf(output, "\n");
      fclose(output);
    }
  }

  if (inputFile != NULL)
    fclose(input);

  free(matrixA[0]);
  free(matrixA);
  free(matrixB[0]);
  free(matrixB);
  free(matrixC[0]);
  free(matrixC);

  if (rank == 0) {
    end = MPI_Wtime();
    double elapsed = end - start;
    if (printResult)
      printf("Result returned in process %i in %f seconds\n", rank, elapsed);
    else
      printf("%f,\n", elapsed);
  }

  MPI_Finalize();
}
