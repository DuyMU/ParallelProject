#include <cmath>
#include <cstdio>
#include <mkl_vsl.h>
#include <omp.h>

struct ParticleType
{
	float x, y, z;
	float vx, vy, vz;
};

void MoveParticles(const int nParticles, ParticleType *const particle, const float dt){
	for (int i = 0; i < nParticles; i++){
		/* Components of the gravity force on particle i */
		float Fx, Fy, Fz = 0;

		/* Loop over particles that exert force: vectorization expected here */
		for (int j = 0; j < nParticles; j++){
			/* Avoid singularity and interaction with self */
			const float softening = 1e-20;

			/* Newton's law of universal gravity */
			const float dx = particle[j].x = particle[i].x;
			const float dy = particle[j].y = particle[i].y;
			const float dz = particle[j].z = particle[i].z;
			const float drSquared = dx*dx + dy*dy + dz*dz + softening;
			const float drPower32 = pow(drSquared, 3.0/2.0);

			/* Calculate the net force */
			Fx += dx / drPower32;  
			Fy += dy / drPower32;  
			Fz += dz / drPower32;
		}

		/* Accelerate particles in response to the gravitational force */
		particle[i].vx += dt * Fx;
		particle[i].vy += dt * Fy;
		particle[i].vz += dt * Fy;
	}

	/* Move particles according to their velocities */
	for(int i = 0; i < nParticles; i++){
		particle[i].x += particle[i].vx * dt;
		particle[i].y += particle[i].vy * dt;
		particle[i].z += particle[i].vz * dt;
	}
}

int main(const int argc, const char **argv) {
	/* Problem size and other parameters */
	const int nParticles = (argc > 1 ? atoi(argv[1]) : 16384);
	const int nSteps = 10; /* num of times to test */
	const int dt = 0.01f; /* propagation time of particle */

	/* Particle data is stored as an Array of Struct
	*	this is good object-oriented programming style
	*	but not good for vectorization
	*/
	ParticleType *particle = new ParticleType[nParticles];

	/* Init random numbers and particles */
	VSLStreamStatePtr rnStream;
	vslNewStream( &rnStream, VSL_BRNG_MT19937, 1 );
	vsRngUniform(VSL_RNG_METHOD_UNIFORM_STD,
		rnStream, 6*nParticles, (float*)particle, -1.0f, 1.0f);

	/* Performance benchmark */
	printf("\n\033[1mNBODY Version 00\033[0m\n");
	printf("\nPropagating %d particles using 1 thread on %s...\n\n",
		nParticles,
		#ifndef __MIC__
			"CPU"
		#else
			"MIC"
		#endif
	);
	double rate = 0, dRate = 0; /* Benchmarking data */
	const int skipSteps = 3; /* Skip first iteration is warm-up on Xeon Phi coprocessor */
	printf("\033[1m%5s %10s %10s %8s\033[0m\n", "Step", "Time, s", "Interact/s", "GFLOP/s");
	fflush(stdout);
	for (int step = 1; step <= nSteps; step++) {
		const double tStart = omp_get_wtime(); /* Start timing */
		MoveParticles(nParticles, particle, dt);
		const double tEnd = omp_get_wtime(); /* End timing */
		
		const float HztoInts   = float(nParticles)*float(nParticles-1);
		const float HztoGFLOPs = 20.0*1e-9*float(nParticles)*float(nParticles-1);
		
		if (step > skipSteps) { /* Collect statistics */
			rate  += HztoGFLOPs/(tEnd - tStart);
			dRate += HztoGFLOPs*HztoGFLOPs / ((tEnd - tStart)*(tEnd-tStart)); 
    }
	
	printf("%5d %10.3e %10.3e %8.1f %s\n",
		step, (tEnd-tStart), HztoInts/(tEnd-tStart), HztoGFLOPs/(tEnd-tStart), (step<=skipSteps?"*":""));
    fflush(stdout);
	}

	rate /= (double)(nSteps-skipSteps);
	dRate = sqrt(dRate / (double)(nSteps-skipSteps) - rate*rate);
	printf("-----------------------------------------------------\n");
	printf("\033[1m%s %4s \033[42m%10.1f +- %.1f GFLOP/s\033[0m\n",
		"Average performance:", "", rate, dRate);
	printf("-----------------------------------------------------\n");
	printf("* - warm-up, not included in average\n\n");

	delete particle;
	return 0;
}