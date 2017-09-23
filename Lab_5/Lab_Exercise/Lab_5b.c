#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define MAXTASKS      8192
#define STARTSIZE     128000
#define ENDSIZE       1024000
#define ROUND		  6

int main (int argc, char *argv[])
{
int     numtasks, rank, n, i, j, rndtrps, nbytes, start, end, incr,
        src, dest, rc, tag=1;
double  thistime, bw, 
		totalbw, avgbw, avgall,
        recv, t1, t2;
char    msgbuf[ENDSIZE];

MPI_Status status;
MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
if (numtasks % 2 != 0) {
  printf("ERROR: Must be an even number of tasks!  Quitting...\n");
  MPI_Abort(MPI_COMM_WORLD, rc);
  return;
}
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
start = STARTSIZE;
end = ENDSIZE;
rndtrps = ROUND;

for (i = 0; i < 4; i++)
  msgbuf[i] = 'x';

/* Determine who my send/receive partner is and tell task 0 */
if (rank < numtasks/2) 
  dest = src = numtasks/2 + rank;
if (rank >= numtasks/2) 
  dest = src = rank - numtasks/2;

if (rank == 0) {
  printf("************************************************************\n");
}

if (rank < numtasks/2) {
  for (n = start; n <= end; n = n * 2) {
    totalbw = 0.0;
    nbytes = sizeof(char) * n;
    for (i=1; i<=rndtrps; i++){
      t1 = MPI_Wtime();
      MPI_Send(&msgbuf, n, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
      MPI_Recv(&msgbuf, n, MPI_CHAR, src, tag, MPI_COMM_WORLD, &status);
      t2 = MPI_Wtime();
	  
      thistime = t2 - t1;
      bw = ((double)nbytes * 2) / thistime;
      totalbw = totalbw + bw;
    }
	
    avgbw = (totalbw/(1024.0*1024))/(double)rndtrps;
    if(rank == 0) {
	  printf("Exchange package %d\n", n);
	  printf("avgbw %4.2f\n", avgbw);   
	
	  double result = avgbw, temp;
	  for (j = 1; j < numtasks/2; j++) {
		MPI_Recv(&temp, 1, MPI_DOUBLE, j, tag, MPI_COMM_WORLD, &status);
		result += temp;
	  }
	  
	  printf("   OVERALL AVERAGES:          %4.2f \n\n", result/(numtasks/2));
    }
    else {
      recv = avgbw;     
      MPI_Send(&recv, 1, MPI_DOUBLE, 0, tag, MPI_COMM_WORLD);
    }
  }
}

if (rank >= numtasks/2) {
  for (n = start; n <= end; n = n * 2) {
    for (i=1; i<=rndtrps; i++){
      MPI_Recv(&msgbuf, n, MPI_CHAR, src, tag, MPI_COMM_WORLD, &status);
      MPI_Send(&msgbuf, n, MPI_CHAR, dest, tag, MPI_COMM_WORLD);
    }
  }
}


MPI_Finalize();

}  /* end of main */
