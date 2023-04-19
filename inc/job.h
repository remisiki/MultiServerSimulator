/**
* Module implementing a job (or request) that comes into the server
* Use JobBuffer to manage an array of jobs.
*/
#ifndef _JOB_H
#define _JOB_H

#include <gsl/gsl_randist.h>
#include <math.h>
#include <stdint.h>
#include "param.h"

// Initial job buffer allocation size
extern const uint32_t INIT_JOB_BUFFER_SIZE;

/**
* Job struct
* @param jobType an integer in [0, JOB_TYPE_CNT) defined in param.h
* @param region an integer in [0, REGION_CNT) defined in param.h
* @param timeToFinish an integer telling remaining time to finish the job
* @param waitTime an integer telling time this job already waited
*/
typedef struct Job {
	uint8_t jobType;
	uint32_t region;
	uint32_t timeToFinish;
	uint32_t waitTime;
} Job;

/**
* A buffer of jobs, including job counts
* @param jobCnt Number of jobs (not NULL) in the buffer
* @param size Size allocated to the struct
*/
typedef struct JobBuffer {
	Job** jobs;
	uint32_t jobCnt;
	uint32_t size;
} JobBuffer;

/**
* Create new jobs in one time unit
* Must init gsl rng first and assign to RNG defined in param.h
* Needs to be freed manually or call freeJobBuffer(). If some job pointers are
* referenced in other places, do not call freeJobBuffer() to avoid conflicts.
*/
JobBuffer newJobs();

/**
* Free a JobBuffer
* This also frees all jobs in the buffer.
*/
void freeJobBuffer(JobBuffer);

/**
* Print out job info
*/
void printJob(Job* job);

#endif
