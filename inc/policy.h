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
*/
uint32_t schedule(Server** servers, const char* policy);

#endif
