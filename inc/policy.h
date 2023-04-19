/**
* Module implementing policies (or queueing algorithms)
* Possible policies:
* - fcfsLocal: First Come First Serve, all jobs are served locally (jobs from
*   region A will not be served in region B)
* - fcfsCross: First Come First Serve, all jobs may be served at a remote
*   server if current region is congested
* - fcfsCrossPart: First Come First Serve, only small jobs may be served at a
*   remote server if current region is congested
* - o3CrossPart: Out Of Order, only small jobs may be served at a remote server
*   if current region is congested and smaller jobs coming late may get served
*   first
* - jsqMaxweight: Proposed in Weina et al. 2016. Push job to join the shortest
*   queue and use Max Weight algorithm to schedule which queue to serve.
*/
#ifndef _POLICY_H
#define _POLICY_H

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_rng.h>
#include "job.h"
#include "queue.h"
#include "server.h"
#include "param.h"

/**
* Schedule servers according to policy, returns a sum of queueing length in one
* time unit (of all regions).
* @param commonQueue Maintain a common queue for all servers. This is for
* jsqMaxweight, keep it null for other policies.
*/
uint32_t schedule(Server** servers, const char* policy, Queue* commonQueue);

#endif
