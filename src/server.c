#include "server.h"

Server* newServer(uint32_t region, uint32_t processorCnt) {
	Server* server = (Server*)malloc(sizeof(Server));
	server->region = region;
	server->processorCnt = processorCnt;
	server->idleCnt = processorCnt;
	Queue* q = newQueue();
	server->waitingQueue = q;
	Job** jobs = (Job**)malloc(0);
	JobBuffer jobBuffer = {jobs, 0};
	server->jobBuffer = jobBuffer;
	return server;
}

void freeServer(Server* server) {
	freeQueue(server->waitingQueue);
	freeJobBuffer(server->jobBuffer);
	free(server);
}

uint32_t calcServiceTime(Server* server, Job* job) {
	uint32_t mean = MEAN_SERVICE_TIME[server->region][job->region];
	// Take the ceil of random number to avoid 0 service time
	uint32_t serviceTime = (uint32_t)ceil(gsl_ran_exponential(RNG, mean));
	return serviceTime;
}

void assignJobToServer(Server* server, Job* job) {
	// TODO Realloc each time is too expensive. Adjust jobBuffer size by doubling
	// if not enough.
	job->timeToFinish = calcServiceTime(server, job);
	server->jobBuffer.jobCnt ++;
	server->jobBuffer.jobs = (Job**)realloc(server->jobBuffer.jobs, server->jobBuffer.jobCnt*sizeof(Job*));
	server->jobBuffer.jobs[server->jobBuffer.jobCnt-1] = job;
	server->idleCnt -= (SERVER_NEEDS[job->jobType]);
}

void serveJobs(Server* server) {
	// TODO Here copies all non-completed jobs to a new array. Not too bad but
	// maybe implementing a linked list will be better?
	Job** newJobs = (Job**)malloc(server->jobBuffer.jobCnt*sizeof(Job*));
	uint32_t newJobCnt = 0;
	for (uint32_t i = 0; i < server->jobBuffer.jobCnt; i ++) {
		Job* job = server->jobBuffer.jobs[i];
		job->timeToFinish --;
		if (job->timeToFinish > 0) {
			newJobs[newJobCnt] = job;
			newJobCnt ++;
		} else {
			server->idleCnt += SERVER_NEEDS[job->jobType];
			// Free the finished job
			free(job);
		}
	}
	newJobs = (Job**)realloc(newJobs, newJobCnt*sizeof(Job*));
	free(server->jobBuffer.jobs);
	server->jobBuffer.jobs = newJobs;
	server->jobBuffer.jobCnt = newJobCnt;
}
