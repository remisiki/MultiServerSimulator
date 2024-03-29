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
	server->departedJobCnt = 0;
	server->departedJobDelay = 0;
	return server;
}

void freeServer(Server* server) {
	freeQueue(server->waitingQueue);
	freeJobBuffer(server->jobBuffer);
	free(server);
}

void assignJobToServer(Server* server, Job* job) {
	// Decay service rate (increase service time)
	job->timeToFinish *= MEAN_SERVICE_TIME[server->region*REGION_CNT+job->region];
	server->jobBuffer.jobCnt ++;
	server->departedJobCnt ++;
	server->departedJobDelay += job->waitTime;
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
	// Walk through the queue and increment job delay by 1
	for (Node* pos = server->waitingQueue->head; pos != NULL; pos = pos->next) {
		pos->job->waitTime ++;
	}
}

uint8_t canServe(Server* server, Job* job) {
	uint8_t jobType = 0;
	if (job != NULL) jobType = job->jobType;
	return (server->idleCnt >= SERVER_NEEDS[jobType]);
}

/**
* Calculate virtual size of a single job
*/
uint32_t calcVirtualQueueSize(Server* server, Job* job) {
	return SERVER_NEEDS[job->jobType]*MEAN_SERVICE_TIME[server->region*REGION_CNT+job->region];
}

void pushQueueVirtual(Server* server, Job* job) {
	pushQueue(server->waitingQueue, job);
	server->waitingQueue->virtualSize += calcVirtualQueueSize(server, job);
}

void removeQueueVirtual(Server* server, Node* node) {
	server->waitingQueue->virtualSize -= calcVirtualQueueSize(server, node->job);
	removeQueue(server->waitingQueue, node);
}

