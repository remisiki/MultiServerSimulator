/**
* Module including all paramters
* These parameters should be initialized or parse in the main module. Include
* this header to share parameters among modules.
*/
#ifndef _PARAM_H
#define _PARAM_H

#include <stdint.h>
#include <gsl/gsl_rng.h>

// GSL rng, must be assigned before any random operations
extern gsl_rng* RNG;

// Simulation time units, default 1E6
// Assign a smaller value for debug and test
extern uint32_t SIMULATION_TIME;

// Processor count for each server, default 48
// All servers will have the same number of processors
// TODO Allow different processor numbers for different servers
extern uint32_t PROC_CNT;

// Job type count, default 2
extern uint8_t JOB_TYPE_CNT;

// Arrival rate array, default {10, 4}
// Must have size of JOB_TYPE_CNT. Arrival count in one time unit follows a
// Poisson distribution given the mean arrival rate. Adjust according to
// PROC_CNT.
extern double* ARRIVAL_RATE;

// Server needs array, default {1, 4}
// Must have size of JOB_TYPE_CNT. Adjust according to PROC_CNT.
extern uint32_t* SERVER_NEEDS;

extern uint32_t REGION_CNT;

extern uint32_t* MEAN_SERVICE_TIME;

#endif
