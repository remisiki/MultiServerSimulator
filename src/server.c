#include "server.h"

Server* newServer(uint32_t region, uint32_t processorCnt) {
	Server* server = (Server*)malloc(sizeof(Server));
	server->region = region;
	server->processorCnt = processorCnt;
	server->idleCnt = processorCnt;
	Queue* q = newQueue();
	server->waitingQueue = q;
	Job** jobs = (Job**)malloc(INIT_JOB_BUFFER_SIZE*sizeof(Job*));
	JobBuffer jobBuffer = {jobs, 0, INIT_JOB_BUFFER_SIZE};
	server->jobBuffer = jobBuffer;
	return server;
}

void freeServer(Server* server) {
	freeQueue(server->waitingQueue);
	freeJobBuffer(server->jobBuffer);
	free(server);
}

uint32_t calcServiceTime(Server* server, Job* job) {
	uint32_t mean = MEAN_SERVICE_TIME[server->region*REGION_CNT+job->region];
	uint32_t serviceTime = (uint32_t)floor(gsl_ran_exponential(RNG, mean));
	return serviceTime;
}

void assignJobToServer(Server* server, Job* job) {
	job->timeToFinish = calcServiceTime(server, job);
	server->jobBuffer.jobCnt ++;
	if (server->jobBuffer.jobCnt > server->jobBuffer.size) {
		if (server->jobBuffer.size == 0) {
			// If is empty, assign init size
			server->jobBuffer.size = INIT_JOB_BUFFER_SIZE;
		} else {
			// Else double the size
			server->jobBuffer.size <<= 1;
		}
		server->jobBuffer.jobs = (Job**)realloc(server->jobBuffer.jobs, server->jobBuffer.size*sizeof(Job*));
	}
	server->jobBuffer.jobs[server->jobBuffer.jobCnt-1] = job;
	server->idleCnt -= (SERVER_NEEDS[job->jobType]);
}

void serveJobs(Server* server) {
	// Here copies all non-completed jobs to a new array. Not too bad since only
	// pointers are moved. Implementing a linked list may cost more to delete all
	// links one by one.
	Job** newJobs = (Job**)malloc(server->jobBuffer.size*sizeof(Job*));
	uint32_t newJobCnt = 0;
	for (uint32_t i = 0; i < server->jobBuffer.jobCnt; i ++) {
		Job* job = server->jobBuffer.jobs[i];
		if (job->timeToFinish > 0) {
			// Ignore when timeToFinish is already zero
			job->timeToFinish --;
		}
		if (job->timeToFinish > 0) {
			newJobs[newJobCnt] = job;
			newJobCnt ++;
		} else {
			server->idleCnt += SERVER_NEEDS[job->jobType];
			// Free the finished job
			free(job);
		}
	}
	// No need to reallocate, since size not changed. Size of buffer will not
	// grow beyond twice the processorCnt, quite safe not to shrink.
	free(server->jobBuffer.jobs);
	server->jobBuffer.jobs = newJobs;
	server->jobBuffer.jobCnt = newJobCnt;
}

uint8_t canServe(Server* server, Job* job) {
	uint8_t jobType = 0;
	if (job != NULL) jobType = job->jobType;
	return (server->idleCnt >= SERVER_NEEDS[jobType]);
}
