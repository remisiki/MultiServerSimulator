/**
* Write all policies here.
* Uncomment printf lines to see verbose results, only do this on small
* simulation iterations. Using gdb is better, but printf is just simple and
* fast.
*/
#include "policy.h"

void fcfsLocal(Server**);

void fcfsCross(Server**);

void fcfsCrossPart(Server**);

void o3CrossPart(Server**);

void jsq(Server**);

void jsqPart(Server**);

void jsqMaxweight(Server**, Queue*);

uint32_t schedule(Server** servers, const char* policy, Queue* commonQueue) {
	uint32_t sumQueueLength = 0;
	if (strcmp(policy, "fcfsLocal") == 0) {
		fcfsLocal(servers);
	} else if (strcmp(policy, "fcfsCross") == 0) {
		fcfsCross(servers);
	} else if (strcmp(policy, "fcfsCrossPart") == 0) {
		fcfsCrossPart(servers);
	} else if (strcmp(policy, "o3CrossPart") == 0) {
		o3CrossPart(servers);
	} else if (strcmp(policy, "jsq") == 0) {
		jsq(servers);
	} else if (strcmp(policy, "jsqPart") == 0) {
		jsqPart(servers);
	} else if (strcmp(policy, "jsqMaxweight") == 0) {
		jsqMaxweight(servers, commonQueue);
	}
	// Serve all jobs in the processors for one time unit and record queue length
	for (uint32_t i = 0; i < REGION_CNT; i ++) {
		Server* server = servers[i];
		serveJobs(server);
		/* printf("Server %d working, remaining idle %d\n", i, server->idleCnt); */
		/* printf("Server %d queue length %d\n", i, getQueueSize(server->waitingQueue)); */
		sumQueueLength += getQueueSize(server->waitingQueue);
	}
	if (commonQueue != NULL) {
		sumQueueLength += getQueueSize(commonQueue);
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
			if (canServe(server, job)) {
				assignJobToServer(server, job);
				popQueue(server->waitingQueue);
			} else {
				// Block the queue if head cannot be served
				break;
			}
		}
	}
	// Create random new jobs
	JobBuffer jobBuffer = newJobs();
	for (uint32_t i = 0; i < jobBuffer.jobCnt; i ++) {
		Job* job = jobBuffer.jobs[i];
		// Only serve the job locally
		Server* server = servers[job->region];
		if (canServe(server, job)) {
			assignJobToServer(server, job);
		} else {
			pushQueue(server->waitingQueue, job);
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
	if (!canServe(servers[job->region], job)) {
		uint32_t minServiceTime = UINT32_MAX;
		for (uint32_t j = 0; j < REGION_CNT; j ++) {
			Server* server = servers[j];
			uint32_t serviceTime = MEAN_SERVICE_TIME[server->region*REGION_CNT+job->region];
			if (
				(canServe(server, job)) &&
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
			// Check the best region that can serve the job
			int bestRegion = getBestRegion(servers, job);
			if (bestRegion != -1) {
				assignJobToServer(servers[bestRegion], job);
				popQueue(server->waitingQueue);
			} else {
				// No region available, block the queue
				break;
			}
		}
	}
	// Create random new jobs
	JobBuffer jobBuffer = newJobs();
	for (uint32_t i = 0; i < jobBuffer.jobCnt; i ++) {
		Job* job = jobBuffer.jobs[i];
		// Also check the best region for new coming jobs
		int bestRegion = getBestRegion(servers, job);
		if (bestRegion == -1) {
			// No region available, push into local quueue
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
				// Small job, check cross region availability
				int bestRegion = getBestRegion(servers, job);
				if (bestRegion != -1) {
					assignJobToServer(servers[bestRegion], job);
					popQueue(server->waitingQueue);
				} else {
					break;
				}
			} else {
				// Large jobs, serve locally
				if (canServe(server, job)) {
					assignJobToServer(server, job);
					popQueue(server->waitingQueue);
				} else {
					break;
				}
			}
		}
	}
	// Create random new jobs
	JobBuffer jobBuffer = newJobs();
	for (uint32_t i = 0; i < jobBuffer.jobCnt; i ++) {
		Job* job = jobBuffer.jobs[i];
		// Same as fcfsCross, but only cross when small jobs
		if (job->jobType == 0) {
			// Small job, check cross region availability
			int bestRegion = getBestRegion(servers, job);
			if (bestRegion == -1) {
				pushQueue(servers[job->region]->waitingQueue, job);
			} else {
				assignJobToServer(servers[bestRegion], job);
			}
		} else {
			// Large jobs, serve locally
			if (canServe(servers[job->region], job)) {
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

void o3CrossPart(Server** servers) {
	// Check whether all regions are full (cannot serve smallest job)
	uint8_t allRegionFull = 1;
	for (uint32_t i = 0; i < REGION_CNT; i ++) {
		if (canServe(servers[i], NULL)) {
			allRegionFull = 0;
			break;
		}
	}
	// Only scan the queue if at least one region is not congested
	if (!allRegionFull) {
		for (uint32_t i = 0; i < REGION_CNT; i ++) {
			Server* server = servers[i];
			// Same as fcfsCrossPart, but iterate through the queue to find all
			// possible jobs that can be served
			Node* pos = server->waitingQueue->head;
			while (pos != NULL) {
				Job* job = pos->job;
				Node* next = pos->next;
				if (job->jobType == 0) {
					int bestRegion = getBestRegion(servers, job);
					if (bestRegion != -1) {
						assignJobToServer(servers[bestRegion], job);
						removeQueue(server->waitingQueue, pos);
					}
				} else {
					if (canServe(server, job)) {
						assignJobToServer(server, job);
						removeQueue(server->waitingQueue, pos);
					}
				}
				pos = next;
			}
		}
	}
	// Create random new jobs
	// Same as fcfsCrossPart
	JobBuffer jobBuffer = newJobs();
	for (uint32_t i = 0; i < jobBuffer.jobCnt; i ++) {
		Job* job = jobBuffer.jobs[i];
		if (job->jobType == 0) {
			int bestRegion = getBestRegion(servers, job);
			if (bestRegion == -1) {
				pushQueue(servers[job->region]->waitingQueue, job);
			} else {
				assignJobToServer(servers[bestRegion], job);
			}
		} else {
			if (canServe(servers[job->region], job)) {
				assignJobToServer(servers[job->region], job);
			} else {
				pushQueue(servers[job->region]->waitingQueue, job);
			}
		}
	}
	free(jobBuffer.jobs);
}

void jsq(Server** servers) {
	// JSQ (virtual queue) routing
	JobBuffer jobBuffer = newJobs();
	for (uint32_t i = 0; i < jobBuffer.jobCnt; i ++) {
		Job* job = jobBuffer.jobs[i];
		uint32_t shortestVirtualQueueLength = UINT32_MAX;
		uint32_t shortestVirtualQueueIndex = 0;
		for (uint32_t j = 0; j < REGION_CNT; j ++) {
			uint32_t virtualQueueLength = servers[j]->waitingQueue->virtualSize;
			if (virtualQueueLength < shortestVirtualQueueLength) {
				shortestVirtualQueueLength = virtualQueueLength;
				shortestVirtualQueueIndex = j;
			}
		}
		pushQueueVirtual(servers[shortestVirtualQueueIndex], job);
	}
	free(jobBuffer.jobs);
	// Scheduling
	for (uint32_t i = 0; i < REGION_CNT; i ++) {
		Server* server = servers[i];
		Node* pos = server->waitingQueue->head;
		while (pos != NULL) {
			Job* job = pos->job;
			Node* next = pos->next;
			if (canServe(server, job)) {
				assignJobToServer(server, job);
				removeQueueVirtual(server, pos);
			} else {
				// Block the queue
				break;
			}
			pos = next;
		}
	}
}

void jsqPart(Server** servers) {
	// Same as jsq, but only route small jobs
	// JSQ (virtual queue) routing
	JobBuffer jobBuffer = newJobs();
	for (uint32_t i = 0; i < jobBuffer.jobCnt; i ++) {
		Job* job = jobBuffer.jobs[i];
		if (job->jobType == 0) {
			uint32_t shortestVirtualQueueLength = UINT32_MAX;
			uint32_t shortestVirtualQueueIndex = 0;
			for (uint32_t j = 0; j < REGION_CNT; j ++) {
				uint32_t virtualQueueLength = servers[j]->waitingQueue->virtualSize;
				if (virtualQueueLength < shortestVirtualQueueLength) {
					shortestVirtualQueueLength = virtualQueueLength;
					shortestVirtualQueueIndex = j;
				}
			}
			pushQueueVirtual(servers[shortestVirtualQueueIndex], job);
		} else {
			pushQueueVirtual(servers[job->region], job);
		}
	}
	free(jobBuffer.jobs);
	// Scheduling
	for (uint32_t i = 0; i < REGION_CNT; i ++) {
		Server* server = servers[i];
		Node* pos = server->waitingQueue->head;
		while (pos != NULL) {
			Job* job = pos->job;
			Node* next = pos->next;
			if (canServe(server, job)) {
				assignJobToServer(server, job);
				removeQueueVirtual(server, pos);
			} else {
				// Block the queue
				break;
			}
			pos = next;
		}
	}
}

void jsqMaxweight(Server** servers, Queue* commonQueue) {
	// JSQ routing
	JobBuffer jobBuffer = newJobs();
	for (uint32_t i = 0; i < jobBuffer.jobCnt; i ++) {
		Job* job = jobBuffer.jobs[i];
		if (getQueueSize(servers[job->region]->waitingQueue) <= getQueueSize(commonQueue)) {
			pushQueue(servers[job->region]->waitingQueue, job);
		} else {
			pushQueue(commonQueue, job);
		}
	}
	free(jobBuffer.jobs);
	// MaxWeight scheduling
	for (uint32_t i = 0; i < REGION_CNT; i ++) {
		Server* server = servers[i];
		// Loop through two queues simultaneously
		while (!(queueIsEmpty(server->waitingQueue) && (queueIsEmpty(commonQueue)))) {
			Node* localHead = server->waitingQueue->head;
			Node* commonHead = commonQueue->head;
			double localWeight = 0;
			double commonWeight = 0;
			if (localHead != NULL) {
				localWeight = (double)getQueueSize(server->waitingQueue)/MEAN_SERVICE_TIME[server->region*REGION_CNT+localHead->job->region];
			}
			if (commonHead != NULL) {
				commonWeight = (double)getQueueSize(commonQueue)/MEAN_SERVICE_TIME[server->region*REGION_CNT+commonHead->job->region];
			}
			// Select the job that has a larger weight
			uint8_t serveLocalJob = (localWeight >= commonWeight);
			Job* job = serveLocalJob ? localHead->job : commonHead->job;
			Queue* queue = serveLocalJob ? server->waitingQueue : commonQueue;
			if (canServe(server, job)) {
				assignJobToServer(server, job);
				popQueue(queue);
			} else {
				// Block the queue
				break;
			}
		}
	}
}
