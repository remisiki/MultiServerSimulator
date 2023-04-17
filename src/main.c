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
uint32_t REGION_CNT;
uint32_t* MEAN_SERVICE_TIME;

/**
* Split a string from source by a delimiter (comma) and store to destination
* NOTE This function modifies destination array but not source array
* NOTE Please specify the number of tokens after splitting, it cannot be larger
* than the actual number. Make sure that destination has enough memory
* allocated before calling this function. destination can only be an array of
* uint32_t or double which stores the number converted from splitted string.
* @param source Source string, does not modify
* @param destination Destination, a pointer to array of uint32_t or double
* @param size Number of tokens after splitting
* @param type Type of destination array. 0 for uint32_t and 1 for double
*/
void split(const char* source, void* destination, uint32_t size, uint8_t type) {
	char* tmp = (char*)malloc((strlen(source)+1)*sizeof(char));
	strcpy(tmp, source);
	if (type == 0) {
		uint32_t* d = (uint32_t*)destination;
		*d = (uint32_t)atoi(strtok(tmp, ","));
		for (uint32_t i = 1; i < size; i ++) {
			*(d+i) = (uint32_t)atoi(strtok(NULL, ","));
		}
	} else if (type == 1) {
		char* eptr;
		double* d = (double*)destination;
		*d = strtod(strtok(tmp, ","), &eptr);
		for (uint32_t i = 1; i < size; i ++) {
			*(d+i) = strtod(strtok(NULL, ","), &eptr);
		}
	}
	free(tmp);
}

int main(int argc, const char* argv[]) {

	// Default parameters
	SIMULATION_TIME = 1E5;
	PROC_CNT = 48;
	JOB_TYPE_CNT = 2;
	REGION_CNT = 2;
	ARRIVAL_RATE = (double*)malloc(REGION_CNT*JOB_TYPE_CNT*sizeof(double));
	ARRIVAL_RATE[0] = 10;
	ARRIVAL_RATE[1] = 4;
	ARRIVAL_RATE[2] = 10;
	ARRIVAL_RATE[3] = 4;
	SERVER_NEEDS = (uint32_t*)malloc(JOB_TYPE_CNT*sizeof(uint32_t));
	SERVER_NEEDS[0] = 1;
	SERVER_NEEDS[1] = 4;
	MEAN_SERVICE_TIME = (uint32_t*)malloc(REGION_CNT*REGION_CNT*sizeof(uint32_t));
	MEAN_SERVICE_TIME[0] = 1;
	MEAN_SERVICE_TIME[1] = 2;
	MEAN_SERVICE_TIME[2] = 2;
	MEAN_SERVICE_TIME[3] = 1;
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
				ARRIVAL_RATE = (double*)realloc(ARRIVAL_RATE, REGION_CNT*JOB_TYPE_CNT*sizeof(double));
				SERVER_NEEDS = (uint32_t*)realloc(SERVER_NEEDS, JOB_TYPE_CNT*sizeof(uint32_t));
			}
		} else if (strcmp(argv[i], "-l") == 0) {
			if (i + 1 < argc) {
				split(argv[i+1], ARRIVAL_RATE, REGION_CNT*JOB_TYPE_CNT, 1);
			}
		} else if (strcmp(argv[i], "-s") == 0) {
			if (i + 1 < argc) {
				split(argv[i+1], SERVER_NEEDS, JOB_TYPE_CNT, 0);
			}
		} else if (strcmp(argv[i], "-r") == 0) {
			if (i + 1 < argc) {
				REGION_CNT = (uint32_t)atoi(argv[i+1]);
				ARRIVAL_RATE = (double*)realloc(ARRIVAL_RATE, REGION_CNT*JOB_TYPE_CNT*sizeof(double));
				MEAN_SERVICE_TIME = (uint32_t*)realloc(MEAN_SERVICE_TIME, REGION_CNT*REGION_CNT*sizeof(uint32_t));
			}
		} else if (strcmp(argv[i], "-a") == 0) {
			if (i + 1 < argc) {
				split(argv[i+1], MEAN_SERVICE_TIME, REGION_CNT*REGION_CNT, 0);
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
			printf("%-20s Specify arrival rate. Must be set together with -j. lambda must have size of regionCnt*jobCnt and is separated by a comma (`,` with no spaces). This represents a 2d array in a 1d array format, where the (i*regionCnt+j)th entry means the arrival rate of job type j for the server in the ith region. default 10,4,10,4\n", "-l [lambda...]");
			printf("%-20s Specify server needs. Must be set together with -j. servers must have size of jobCnt and is separated by a comma (`,` with no spaces). default 1,4\n", "-s [servers...]");
			printf("%-20s Specify region number as regionCnt. Must be set before (and together with) -a. Must be set before -l. default 2\n", "-r regionCnt");
			printf("%-20s Specify mean service time across regions. Must be set together with -r. serviceTime must have size of regionCnt^2 and is separated by a comma (`,` with no spaces). This represents a 2d array in a 1d array format, where the (i*regionCnt+j)th entry means the mean service time for the server in the ith region to serve the job from the jth region. default 1,2,2,1\n", "-a [serviceTime...]");
			printf("%-20s Run simulation verbosely.\n", "-v");
			free(ARRIVAL_RATE);
			free(SERVER_NEEDS);
			free(MEAN_SERVICE_TIME);
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
		for (uint8_t i = 0; i < REGION_CNT*JOB_TYPE_CNT; i ++) {
			printf("%lf ", ARRIVAL_RATE[i]);
		}
		printf("\n");
		printf("Server needs: ");
		for (uint8_t i = 0; i < JOB_TYPE_CNT; i ++) {
			printf("%d ", SERVER_NEEDS[i]);
		}
		printf("\n");
		printf("Mean service time across servers: ");
		for (uint8_t i = 0; i < REGION_CNT*REGION_CNT; i ++) {
			printf("%d ", MEAN_SERVICE_TIME[i]);
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
	// For the queueing delay metric, only count jobs that already departed,
	// since those still in the queue have unknown final waitTime.
	uint32_t sumDepartedJobCnt = 0;
	uint32_t sumDepartedJobDelay = 0;
	for (uint32_t i = 0; i < REGION_CNT; i ++) {
		sumDepartedJobCnt += servers[i]->departedJobCnt;
		sumDepartedJobDelay += servers[i]->departedJobDelay;
	}
	double expectedJobDelay = (double)sumDepartedJobDelay/sumDepartedJobCnt;
	if (verbose) {
		printf("\n");
		printf("Stop simulation\n");
	}
	if (verbose) {
		printf("Expected queue length: %lf\n", expectedQueueLength);
		printf("Expected queueing delay: %lf\n", expectedJobDelay);
	} else {
		printf("%lf\n", expectedQueueLength);
		printf("%lf\n", expectedJobDelay);
	}

	// Cleanup
	for (uint32_t i = 0; i < REGION_CNT; i ++) {
		freeServer(servers[i]);
	}
	free(servers);
	gsl_rng_free(RNG);
	free(ARRIVAL_RATE);
	free(SERVER_NEEDS);
	free(MEAN_SERVICE_TIME);

	return 0;
}
