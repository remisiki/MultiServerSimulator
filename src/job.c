#include "job.h"

JobBuffer newJobs() {
	uint32_t jobCnt = 0;
	Job** jobs = (Job**)malloc(jobCnt);
	for (uint32_t i = 0; i < REGION_CNT; i ++) {
		for (uint8_t j = 0; j < JOB_TYPE_CNT; j ++) {
			uint32_t arrivingCnt = gsl_ran_poisson(RNG, ARRIVAL_RATE[j]);
			jobs = (Job**)realloc(jobs, (jobCnt+arrivingCnt)*sizeof(Job*));
			for (uint32_t k = 0; k < arrivingCnt; k ++) {
				Job* job = (Job*)malloc(sizeof(Job));
				job->jobType = j;
				job->region = i;
				jobs[jobCnt+k] = job;
			}
			jobCnt += arrivingCnt;
		}
	}
	// Shuffle the jobs (closer to reality)
	gsl_ran_shuffle(RNG, jobs, jobCnt, sizeof(Job*));

	JobBuffer jobBuffer;
	jobBuffer.jobs = jobs;
	jobBuffer.jobCnt = jobCnt;
	return jobBuffer;
}

void freeJobBuffer(JobBuffer jobBuffer) {
	for (uint32_t i = 0; i < jobBuffer.jobCnt; i ++) {
		free(jobBuffer.jobs[i]);
	}
	free(jobBuffer.jobs);
}

void printJob(Job* job) {
	printf("job from region %d, type %d, time to finish %d\n", job->region, job->jobType, job->timeToFinish);
}
