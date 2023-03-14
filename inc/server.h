/**
* Module implementing a server
*/
#ifndef _SERVER_H
#define _SERVER_H

#include <stdint.h>
#include <stdlib.h>
#include <gsl/gsl_randist.h>
#include <math.h>
#include "queue.h"
#include "job.h"
#include "param.h"

/**
* Server struct
* @param region an integer in [0, REGION_CNT) defined in param.h
* @param processorCnt an integer equals to PROC_CNT defined in param.h
* @param idleCnt an integer that tells count of idle processors
* @param waitingQueue a queue that includes jobs waiting to be serverd
* @param jobBuffer a job buffer for all jobs that are being served
*/
typedef struct Server {
	uint32_t region;
	uint32_t processorCnt;
	uint32_t idleCnt;
	Queue* waitingQueue;
	JobBuffer jobBuffer;
} Server;

/**
* Create a new server given region id and processor count
* Needs to be freed by calling freeServer().
*/
Server* newServer(uint32_t region, uint32_t processorCnt);

/**
* Free a server
* This also frees the waiting queue as well as the job buffer (all pending and
* ongoing jobs will be freed.
*/
void freeServer(Server* server);

/**
* Assign a job to the server
* This force the server serve the job by adding it to the jobBuffer, do not
* call if server has not enough idle processors but add it to the waiting queue
* instead.
*/
void assignJobToServer(Server* server, Job* job);

/**
* Serve ongoing jobs for one time unit
* This function only take affects on job buffer and not considering waiting
* queue. Finished jobs will be eliminated and freed.
*/
void serveJobs(Server* server);

/**
* Determine whether a server can serve the job
* If job is a NULL, compare against smallest job type.
*/
uint8_t canServe(Server* server, Job* job);

#endif
