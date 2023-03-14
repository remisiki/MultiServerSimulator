/**
* Module implementing a job (or request) that comes into the server
* Use JobBuffer to manage an array of jobs.
*/
#ifndef _JOB_H
#define _JOB_H

#include <stdint.h>
#include <gsl/gsl_randist.h>
#include "param.h"

/**
* Job struct
* @param jobType an integer in [0, JOB_TYPE_CNT) defined in param.h
* @param region an integer in [0, REGION_CNT) defined in param.h
* @param timeToFinish an integer telling remaining time to finish the job
*/
typedef struct Job {
	uint8_t jobType;
	uint32_t region;
	uint32_t timeToFinish;
} Job;

/**
* A buffer of jobs, including job counts
*/
typedef struct JobBuffer {
	Job** jobs;
	uint32_t jobCnt;
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
