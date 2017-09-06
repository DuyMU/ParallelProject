#include "pi.h"
#include <omp.h>

#define SIZE 200000000
#define CHUNKSIZE 100
#define OMP_NUM_THREADS 48  
double *rect, *midPt, *area;

/* TODO */
/* Modify this function to implement a parallel version with openmp */
int compute(long int num_steps)
{
    double pi;

    double width = 2. / num_steps;
    pi = 0.;
	
	int chunk, i;
	chunk = CHUNKSIZE;
	#pragma omp parallel shared(rect, midPt, chunk) private(i)
	{
		omp_set_num_threads(OMP_NUM_THREADS);
		#pragma omp for schedule(dynamic,chunk) nowait
		for (i = 0; i < num_steps; ++i) 
		{
			rect[i] = (double)i;
            		midPt[i] = (i + 0.5) * width - 1.0;
			area[i] = sqrt(1.0 - midPt[i] * midPt[i]) * width;
		}
	}

	/*#pragma omp parallel shared(area, midPt, chunk) private(i)
	{
		//omp_set_num_threads(OMP_NUM_THREADS);
		#pragma omp for schedule(dynamic,chunk) nowait
		for (i = 0; i < num_steps; ++i)
		{
			area[i] = sqrt(1.0 - midPt[i] * midPt[i]) * width;
		}
	}

	/*#pragma omp parallel shared(area, chunk) private(i, pi)
	{
		//omp_set_num_threads(OMP_NUM_THREADS);
		#pragma omp for schedule(dynamic,chunk) nowait
		for (i = 0; i < num_steps; ++i)
		{
			//#pragma omp critical 
			//{
				pi += area[i] * 2.0;
			//}
		}
	}*/
		
	
    /*for (int i = 0; i < num_steps; ++i)
    {
            rect[i] = (double)i;
            midPt[i] = (i + 0.5) * width - 1.0;
    }

    for (int i = 0; i < num_steps; ++i)
    {
            area[i] = sqrt(1.0 - midPt[i] * midPt[i]) * width;
    }*/

    for (i = 0; i < num_steps; ++i)
    {
            pi += area[i] * 2.0;
    }
	
    std::cout << "PI:" << pi << std::endl;

    return (0);
}

int prepare(long int Count)
{
    int i, j, n = Count;
    rect = new double[Count];
    midPt = new double[Count];
    area = new double[Count];

    return (0);
}

int cleanup(long int N)
{
    delete rect;
    delete midPt;
    delete area;
    return (0);
}

int main(int argc, char *argv[])
{
    long int Count = SIZE;
    int Error;

    if (argc > 1)
    {
            Count = std::atoi(argv[1]);
            if (Count <= 0)
            {
                        std::cerr << "Invalid argument" << std::endl;
                        std::cerr << "Usage: " << argv[0] << "N" << std::endl;
                        std::cerr << "       N = size" << std::endl;
                        return 1;
            }
    }

    std::cout << "counts:" << Count << std::endl;
    std::cout << "preparation starting" << std::endl;
    if (Error = prepare(Count) != 0)
        return Error;
    std::cout << "preparation done" << std::endl;
    unsigned long long start_ticks = my_getticks();
    Error = compute(Count);

    unsigned long long end_ticks = my_getticks();
    unsigned long long ticks = end_ticks - start_ticks;

    if (Error == 0)
        std::cout << "succeeded in ";
    else
        std::cout << "failed in ";
    std::cout << my_ticks_to_seconds(ticks) << " seconds." << std::endl;
    std::cout << "starting cleanup" << std::endl;
    return cleanup(Count);
}
