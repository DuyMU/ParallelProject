#include <stdio.h>
#include <stdlib.h>
#include "mpi.h"

int main( int argc, char* argv[] )
{
	int rank, nproc;
	int isend, irecv;
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &nproc );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	
	isend = rank + 1;
	MPI_Reduce(&isend, &irecv, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);
	
	if(rank == 0) {
		if (nproc > 1)printf("SUM of 1 + .. + %d = %d\n", nproc, irecv);
		else if(nproc == 1) printf("SUM of 1 = %d\n", irecv);
	}
		
	MPI_Finalize();
	
	return 0;
}
