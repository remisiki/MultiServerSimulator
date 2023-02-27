/**
* Write all policies here.
* Uncomment printf lines to see verbose results, only do this on small
* simulation iterations. Using gdb is better, but printf is just simple and
* fast.
* TODO Implement fcfsCross
* TODO Implement fcfsCrossPart
* TODO Implement o3CrossPart
* TODO Add mean waiting time to the return value
*/
#include "policy.h"

uint32_t fcfsLocal(Server** servers) {
	uint32_t sumQueueLength = 0;
	// First check jobs in the waiting queue. This ensures jobs arriving earlier
	// than the next iteration but in the queue priors to get served.
	for (uint32_t i = 0; i < REGION_CNT; i ++) {
		Server* server = servers[i];
		while (!queueIsEmpty(server->waitingQueue)) {
			Job* job = server->waitingQueue->head->job;
			if (server->idleCnt >= SERVER_NEEDS[job->jobType]) {
				assignJobToServer(server, job);
				/* printf("Assign job of type %d to server %d from queue, remaining idle %d\n", job->jobType, i, server->idleCnt); */
				popQueue(server->waitingQueue);
			} else {
				break;
			}
		}
	}
	// Create random new jobs
	JobBuffer jobBuffer = newJobs();
	/* printf("%d\n", jobBuffer.jobCnt); */
	for (uint32_t i = 0; i < jobBuffer.jobCnt; i ++) {
		Job* job = jobBuffer.jobs[i];
		/* printJob(jobBuffer.jobs[i]); */
		// Loop through all regions in order.
		// TODO This policy does not include cross region operations, but all
		// others do. How to simulate when multi jobs from different regions come
		// to the same server? This is tricky in a single thread program.
		for (uint32_t j = 0; j < REGION_CNT; j ++) {
			Server* server = servers[j];
			if (server->region == job->region) {
				if (server->idleCnt >= SERVER_NEEDS[job->jobType]) {
					assignJobToServer(server, job);
					/* printf("Assign job of type %d to server %d, remaining idle %d\n", job->jobType, j, server->idleCnt); */
				} else {
					pushQueue(server->waitingQueue, job);
					/* printf("Server %d full, current queue length %d\n", j, getQueueSize(server->waitingQueue)); */
				}
			}
		}
	}
	// OK to free the pointer to new jobs, as all jobs are either pushed into
	// processors or waiting queue, which will be freed when servers are freed
	free(jobBuffer.jobs);
	// Serve all jobs in the processors for one time unit and record queue length
	for (uint32_t i = 0; i < REGION_CNT; i ++) {
		Server* server = servers[i];
		serveJobs(server);
		/* printf("Server %d working, remaining idle %d\n", i, server->idleCnt); */
		/* printf("Server %d queue length %d\n", i, getQueueSize(server->waitingQueue)); */
		sumQueueLength += getQueueSize(server->waitingQueue);
	}
	return sumQueueLength;
}
