#include “mpi.h”

int main( int argc, char* argv[] )
{
	int rank, nproc;
	int isend, irecv;
	MPI_Init( &argc, &argv );
	MPI_Comm_size( MPI_COMM_WORLD, &nproc );
	MPI_Comm_rank( MPI_COMM_WORLD, &rank );
	
	isend = rank + 1;
	MPI_Reduce(&isend, &irecv, 1, MPI_INTEGER, MPI_SUM, 0, MPI_COMM_WORLD);
	
	if(rank == 0) printf(“irecv = %d\n”, irecv);
	MPI_Finalize();
	
	return 0;
}