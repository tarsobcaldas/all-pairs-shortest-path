# all-pairs-shortest-path
Gives the solution for finding the shortest distance between all pairs of nodes of a weighed graph
using Message Passing Interface (MPI).

```
  Usage: mpirun -np <processes> shortest-path [options]
      -i <file> --input <file>    Set matrix input file
      -o <file> --output <file>   Specify custom output name
      -h --help                   Display this help message
      -t --timing-only            Don't print the solution, only the timing
      -r --random-matrix <size>   Use random matrix as input
                                  (only if no input file provided)
```
