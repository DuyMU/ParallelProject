#include <sys/time.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "mpi.h"

#define NUM_POINT 10000
#define NUM_PROC 8
#define SEED 35791246

static inline unsigned long long my_getticks(){
    struct timeval t;
    gettimeofday(&t, 0);
    return t.tv_sec * 1000000ULL + t.tv_usec;
}

static inline double my_ticks_to_seconds(unsigned long long ticks){
    return ticks * 1.0e-6;
}

double rand_val(double lower, double upper)
{
	int rand_int = 0;
	double rand_float = 0, max_float = 0, PI = 0;
	
	rand_int = rand();
	rand_float = (double)rand_int;
	max_float = (double)RAND_MAX;
	PI = (rand_float/max_float) * (upper - lower) + lower;
	return PI;
}

int main(int argc, char **argv)
{
	double rand_x, rand_y, rand_xy, piD4, PI = 0;
	int numtasks, rank, len, partner, message, i, inCircle = 0;
	char hostname[MPI_MAX_PROCESSOR_NAME];
	MPI_Status status;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
	long int numPoint = NUM_POINT;
	int numProc = numtasks;
	
	if (argc > 1)
    {
            numPoint = atoi(argv[1]);
            if (numPoint <= 0)
            {
                        printf("Invalid argument: %s\n", argv[0]);
                        return 1;
            }
    }

	long int pointPerProcess = numPoint / numProc;
	if (rank == 0) 
	{
		printf("Number of point: %d\n", numPoint);
    	printf("Preparation done\n");
	}
	unsigned long long start_ticks = my_getticks();
	
	srand((unsigned) time(NULL));
	for (i = 0; i < pointPerProcess; i++) {
		rand_x = (double)rand_val(0.,1.);
		rand_y = (double)rand_val(0.,1.);
		rand_xy = rand_x * rand_x + rand_y * rand_y;
		
		if (rand_xy <= 1.0) {
			inCircle++;
		}
	}
	
	int irecv[numProc];
	int numRecv;
	if (rank != 0) {
		MPI_Send(&inCircle, 1, MPI_INT, 0, 1, MPI_COMM_WORLD);
	}
	else if(rank == 0) {
		PI = inCircle;
		for (i = 1; i < numProc; i++) {
			MPI_Recv(&numRecv, 1, MPI_INT, i, 1, MPI_COMM_WORLD, &status);
			//irecv[i] = numRecv;
			PI += numRecv;
		}
		
		PI = (double) (PI * 4) / numPoint;
		printf("PI: %g\n", PI);
		unsigned long long end_ticks = my_getticks();
    	unsigned long long ticks = end_ticks - start_ticks;
		printf("%g seconds. \n", my_ticks_to_seconds(ticks));
	}
	
	MPI_Finalize();

	return(0);
}
