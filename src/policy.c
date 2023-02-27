/**
* Write all policies here.
* Uncomment printf lines to see verbose results, only do this on small
* simulation iterations. Using gdb is better, but printf is just simple and
* fast.
* TODO Why fcfsCross and fcfsCrossPart performs much worse?
* TODO Implement o3CrossPart
* TODO Add mean waiting time to the return value
*/
#include "policy.h"

void fcfsLocal(Server**);

void fcfsCross(Server**);

void fcfsCrossPart(Server**);

uint32_t schedule(Server** servers, const char* policy) {
	uint32_t sumQueueLength = 0;
	if (strcmp(policy, "fcfsLocal") == 0) {
		fcfsLocal(servers);
	} else if (strcmp(policy, "fcfsCross") == 0) {
		fcfsCross(servers);
	} else if (strcmp(policy, "fcfsCrossPart") == 0) {
		fcfsCrossPart(servers);
	}
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

void fcfsLocal(Server** servers) {
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
}

/**
* Get the best region that can serve the job
* Check through all servers and find the one that has the smallest mean service
* time and is idle. If no available servers can be found, return -1.
*/
int getBestRegion(Server** servers, Job* job) {
	int bestRegion = -1;
	if (servers[job->region]->idleCnt < SERVER_NEEDS[job->jobType]) {
		uint32_t minServiceTime = UINT32_MAX;
		for (uint32_t j = 0; j < REGION_CNT; j ++) {
			Server* server = servers[j];
			uint32_t serviceTime = MEAN_SERVICE_TIME[server->region][job->region];
			if (
				(server->idleCnt >= SERVER_NEEDS[job->jobType]) &&
				(serviceTime < minServiceTime)
			) {
				bestRegion = (int)server->region;
				minServiceTime = serviceTime;
			}
		}
	} else {
		bestRegion = (int)job->region;
	}
	return bestRegion;
}

void fcfsCross(Server** servers) {
	// First check jobs in the waiting queue. This ensures jobs arriving earlier
	// than the next iteration but in the queue priors to get served.
	for (uint32_t i = 0; i < REGION_CNT; i ++) {
		Server* server = servers[i];
		while (!queueIsEmpty(server->waitingQueue)) {
			Job* job = server->waitingQueue->head->job;
			int bestRegion = getBestRegion(servers, job);
			if (bestRegion != -1) {
				/* printf("%d %d\n", bestRegion, servers[bestRegion]->idleCnt); */
			/* } */
			/* if (server->idleCnt >= SERVER_NEEDS[job->jobType]) { */
				assignJobToServer(servers[bestRegion], job);
				/* assignJobToServer(server, job); */
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
		int bestRegion = getBestRegion(servers, job);
		/* if ((bestRegion != -1) && (bestRegion != (int)job->region)) { */
		/* 	printf("Assigning job from region %d to region %d, server0 queue length %d, server1 queue length %d\n", job->region, bestRegion, servers[0]->waitingQueue->size, servers[1]->waitingQueue->size); */
		/* } */
		if (bestRegion == -1) {
			pushQueue(servers[job->region]->waitingQueue, job);
		} else {
			assignJobToServer(servers[bestRegion], job);
		}
	}
	// OK to free the pointer to new jobs, as all jobs are either pushed into
	// processors or waiting queue, which will be freed when servers are freed
	free(jobBuffer.jobs);
}

void fcfsCrossPart(Server** servers) {
	// First check jobs in the waiting queue. This ensures jobs arriving earlier
	// than the next iteration but in the queue priors to get served.
	for (uint32_t i = 0; i < REGION_CNT; i ++) {
		Server* server = servers[i];
		while (!queueIsEmpty(server->waitingQueue)) {
			Job* job = server->waitingQueue->head->job;
			// Same as fcfsCross, but only cross when small jobs
			if (job->jobType == 0) {
				int bestRegion = getBestRegion(servers, job);
				if (bestRegion != -1) {
					assignJobToServer(servers[bestRegion], job);
					/* printf("Assign job of type %d to server %d from queue, remaining idle %d\n", job->jobType, i, server->idleCnt); */
					popQueue(server->waitingQueue);
				} else {
					break;
				}
			} else {
				if (server->idleCnt >= SERVER_NEEDS[job->jobType]) {
					assignJobToServer(server, job);
					/* printf("Assign job of type %d to server %d from queue, remaining idle %d\n", job->jobType, i, server->idleCnt); */
					popQueue(server->waitingQueue);
				} else {
					break;
				}
			}
		}
	}
	// Create random new jobs
	JobBuffer jobBuffer = newJobs();
	/* printf("%d\n", jobBuffer.jobCnt); */
	for (uint32_t i = 0; i < jobBuffer.jobCnt; i ++) {
		Job* job = jobBuffer.jobs[i];
		/* printJob(jobBuffer.jobs[i]); */
		// Same as fcfsCross, but only cross when small jobs
		if (job->jobType == 0) {
			int bestRegion = getBestRegion(servers, job);
			if (bestRegion == -1) {
				pushQueue(servers[job->region]->waitingQueue, job);
			} else {
				assignJobToServer(servers[bestRegion], job);
			}
		} else {
			if (servers[job->region]->idleCnt >= SERVER_NEEDS[job->jobType]) {
				assignJobToServer(servers[job->region], job);
			} else {
				pushQueue(servers[job->region]->waitingQueue, job);
			}
		}
	}
	// OK to free the pointer to new jobs, as all jobs are either pushed into
	// processors or waiting queue, which will be freed when servers are freed
	free(jobBuffer.jobs);
}
