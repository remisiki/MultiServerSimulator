/**
* Main module for simulation
* See README for detailed options.
* TODO Implement all policies (see policy.c)
* TODO Other minor improvements
*/
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <gsl/gsl_rng.h>
#include <immintrin.h>
#include "policy.h"
#include "param.h"

// Define extern paramters in param.h
gsl_rng* RNG;
uint32_t SIMULATION_TIME;
uint32_t PROC_CNT;
uint8_t JOB_TYPE_CNT;
double* ARRIVAL_RATE;
uint32_t* SERVER_NEEDS;

int main(int argc, char const *argv[]) {

	// Default parameters
	SIMULATION_TIME = 1E5;
	PROC_CNT = 48;
	JOB_TYPE_CNT = 2;
	ARRIVAL_RATE = (double*)malloc(JOB_TYPE_CNT*sizeof(double));
	ARRIVAL_RATE[0] = 10;
	ARRIVAL_RATE[1] = 4;
	SERVER_NEEDS = (uint32_t*)malloc(JOB_TYPE_CNT*sizeof(uint32_t));
	SERVER_NEEDS[0] = 1;
	SERVER_NEEDS[1] = 4;
	char POLICY[20] = "fcfsLocal";

	// Parse arguments
	uint8_t verbose = 0;
	for (int i = 1; i < argc; i ++) {
		if (strcmp(argv[i], "-t") == 0) {
			if (i + 1 < argc) {
				SIMULATION_TIME = (uint32_t)atoi(argv[i+1]);
			}
		} else if (strcmp(argv[i], "-n") == 0) {
			if (i + 1 < argc) {
				PROC_CNT = (uint32_t)atoi(argv[i+1]);
			}
		} else if (strcmp(argv[i], "-j") == 0) {
			if (i + 1 < argc) {
				JOB_TYPE_CNT = (uint8_t)atoi(argv[i+1]);
				ARRIVAL_RATE = (double*)realloc(ARRIVAL_RATE, JOB_TYPE_CNT*sizeof(double));
				SERVER_NEEDS = (uint32_t*)realloc(SERVER_NEEDS, JOB_TYPE_CNT*sizeof(uint32_t));
			}
		} else if (strcmp(argv[i], "-l") == 0) {
			if (i + 1 < argc) {
				char* tmp = (char*)malloc((strlen(argv[i+1])+1)*sizeof(char));
				strcpy(tmp, argv[i+1]);
				char* eptr;
				ARRIVAL_RATE[0] = strtod(strtok(tmp, ","), &eptr);
				for (int j = 1; j < JOB_TYPE_CNT; j ++) {
					ARRIVAL_RATE[j] = strtod(strtok(NULL, ","), &eptr);
				}
				free(tmp);
			}
		} else if (strcmp(argv[i], "-s") == 0) {
			if (i + 1 < argc) {
				char* tmp = (char*)malloc((strlen(argv[i+1])+1)*sizeof(char));
				strcpy(tmp, argv[i+1]);
				SERVER_NEEDS[0] = (uint32_t)atoi(strtok(tmp, ","));
				for (int j = 1; j < JOB_TYPE_CNT; j ++) {
					SERVER_NEEDS[j] = (uint32_t)atoi(strtok(NULL, ","));
				}
				free(tmp);
			}
		} else if (strcmp(argv[i], "-p") == 0) {
			if (i + 1 < argc) {
				strcpy(POLICY, argv[i+1]);
			}
		} else if (strcmp(argv[i], "-v") == 0) {
			verbose = 1;
		} else if (strcmp(argv[i], "-h") == 0) {
			printf("MultiServerSimulator\nOptions:\n");
			printf("%-20s Show this help message.\n", "-h");
			printf("%-20s Specify policy from fcfsLocal, fcfsCross, fcfsCrossPart, o3CrossPart. default fcfsLocal\n", "-p");
			printf("%-20s Specify a simulation iteration of time units. default 100000\n", "-t time");
			printf("%-20s Specify number of processors for each server to be num. This will force all servers to have the same number. default 48\n", "-n num");
			printf("%-20s Specify job type count as jobCnt. Must be set before (and together with) -l and -s. default 2\n", "-j jobCnt");
			printf("%-20s Specify arrival rate. Must be set together with -j. lambda must have size of jobCnt and is separated by a comma (,, with no spaces). default 10,4\n", "-l [lambda...]");
			printf("%-20s Specify server needs. Must be set together with -j. servers must have size of jobCnt and is separated by a comma (,, with no spaces). default 1,4\n", "-s [servers...]");
			printf("%-20s Run simulation verbosely.\n", "-v");
			free(ARRIVAL_RATE);
			free(SERVER_NEEDS);
			return 0;
		}
	}

	// Print parameters for confirmation
	if (verbose) {
		printf("Running with parameters:\n");
		printf("Simulation time units: %d\n", SIMULATION_TIME);
		printf("Processor count per server: %d\n", PROC_CNT);
		printf("Job type count: %d\n", JOB_TYPE_CNT);
		printf("Arriving rate: ");
		for (uint8_t i = 0; i < JOB_TYPE_CNT; i ++) {
			printf("%lf ", ARRIVAL_RATE[i]);
		}
		printf("\n");
		printf("Server needs: ");
		for (uint8_t i = 0; i < JOB_TYPE_CNT; i ++) {
			printf("%d ", SERVER_NEEDS[i]);
		}
		printf("\n");
		printf("Policy: %s\n", POLICY);
	}
	/* return 0; */

	// Generate seed
	uint32_t seed;
	_rdrand32_step(&seed);

	// Init rng
	const gsl_rng_type* T;
	gsl_rng_env_setup();
	T = gsl_rng_default;
	RNG = gsl_rng_alloc(T);
	gsl_rng_set(RNG, seed);

	// Start simulation
	if (verbose) {
		printf("Start simulation\n");
	}
	// Create servers
	Server** servers = (Server**)malloc(REGION_CNT*sizeof(Server*));
	for (uint32_t i = 0; i < REGION_CNT; i ++) {
		servers[i] = newServer(i, PROC_CNT);
	}
	// Simulate by time units
	double expectedQueueLength = 0;
	for (uint32_t timestamp = 0; timestamp < SIMULATION_TIME; timestamp ++) {
		if (verbose) printf("%d/%d\r", timestamp+1, SIMULATION_TIME);
		expectedQueueLength += schedule(servers, POLICY);
	}
	expectedQueueLength /= (SIMULATION_TIME*REGION_CNT);
	if (verbose) {
		printf("\n");
		printf("Stop simulation\n");
	}
	if (verbose) {
		printf("Expected queue length: %lf\n", expectedQueueLength);
	} else {
		printf("%lf\n", expectedQueueLength);
	}

	// Cleanup
	for (uint32_t i = 0; i < REGION_CNT; i ++) {
		freeServer(servers[i]);
	}
	free(servers);
	gsl_rng_free(RNG);
	free(ARRIVAL_RATE);
	free(SERVER_NEEDS);

	return 0;
}
